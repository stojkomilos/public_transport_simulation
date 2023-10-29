#include "Moovit.h"
#include"MoovitException.h"

#include<fstream>
#include<iostream>
#include<queue>
#include<assert.h>

#include<string>
#include<iostream>
#include<istream>

using namespace std;

void Moovit::findRoute()
{
	cout << "Molimo izaberite opciju rutom kog tipa zelite da se krecete:" << endl;
	cout << "1. Bilo koja ruta" << endl;
	cout << "2. Ruta sa najmanje presedanja" << endl;
	cout << "3. Ruta koja obilazi sva \"bitna\" mesta u gradu" << endl;

	string routeType;

#ifdef P_DEBUG_FILE
	routeType = "2";
#else
#endif
	getline(cin, routeType);

	stringstream output;
	try
	{
		auto [startingStop, endingStop] = findRouteGeneralPre();

		if (routeType == "1" || routeType == "2") output = findRouteLeastTransfer(startingStop, endingStop); // the same algorithms is used for both findind any path the the path with the least transfers
		else if (routeType == "3") output = findRouteImportant(startingStop, endingStop);
		else throw MoovitExceptionInvalidRouteType(routeType.c_str());

		findRouteGeneralPost(startingStop, endingStop, output);
	}
	catch (MoovitExceptionNoRoute ex)
	{
		cout << "Nije moguca ruta po trazenim zahtevima. " << ex.what() << endl;
	}
	catch (MoovitExceptionBusStop ex)
	{
		cout << "Greska, nevalidna stanica " << ex.what() << endl;
	}
	catch (MoovitExceptionBusLine ex)
	{
		cout << "Greska, nevalidna linija " << ex.what()  << endl;
	}
	catch (MoovitExceptionInvalidRouteType ex)
	{
		cout << "Greska, nevalidan tip rute " << ex.what()  << endl;
	}
}


stringstream Moovit::findRouteImportant(const string& startingStop, const string& endingStop)
{
	if(mIsImportantCode.size() == 0)
		return findRouteLeastTransfer(startingStop, endingStop);

	stringstream output;
	vector<std::string> importantStops;

	for (auto& i : mIsImportantCode)
		importantStops.push_back(i.first);

	for (int i=0; i<importantStops.size(); i++)
	{
		if (i == 0)
			output = findRouteLeastTransfer(startingStop, importantStops[i]);
		else output << findRouteLeastTransfer(importantStops[i-1], importantStops[i]).rdbuf();
	}

	output << findRouteLeastTransfer(importantStops[importantStops.size() - 1], endingStop).rdbuf(); // TODO Proveriti ako ima samo 1 ili 2 ili 0 imp. stop

	return output;
}


std::tuple<std::string, std::string> Moovit::findRouteGeneralPre()
{
	string startingStop;
	string endingStop;

	cout << "Molimo vas unesite sifru pocetne stanice" << endl;
	getline(cin, startingStop);

	cout << "Molimo vas unesite sifru krajnje stanice" << endl;
	getline(cin, endingStop);

	if (mCodeToName.count(startingStop) == 0)
		throw MoovitExceptionBusStop(startingStop.c_str());

	if (mCodeToName.count(endingStop) == 0)
		throw MoovitExceptionBusStop(endingStop.c_str());

	if (startingStop == endingStop)
		throw MoovitExceptionNoRoute("Pocetna stanica je ista kao i krajnja");

	return { startingStop, endingStop };
}

void Moovit::findRouteGeneralPost(const std::string& startingStop, const std::string& endingStop, const std::stringstream& output)
{
	string outputFileName = "putanja_" + startingStop + "_" + endingStop + ".txt";

	ofstream outputFile;
	outputFile.open(outputFileName);

	ostream* outputPlace;
#ifdef P_DEBUG_FILE
	outputPlace = &cout;
#else
	outputPlace = &outputFile;
#endif

	if (outputFile.is_open() == false)
	{
		throw MoovitExceptionFile(outputFileName.c_str());
	}

	(*outputPlace) << output.rdbuf(); // .rdbuf() is used so the output stream can read out the stream contents on the left side
	cout << "Generisan je fajl " << outputFileName << " sa putanjom koju ste izabrali." << endl;

	outputFile.close();
}

stringstream Moovit::findRouteLeastTransfer(const string& startingStop, const string& endingStop)
{
	stringstream output;

	std::map<std::string, bool> visitedMap;

	queue<vector<PartOfPath>> qu;

	vector<PartOfPath> initialPath;
	initialPath.emplace_back("", startingStop);
	qu.push(initialPath);
	visitedMap[startingStop] = false;

	while (qu.size() != 0)
	{
		const vector<PartOfPath>& front = qu.front();

		string currentStop = front[front.size() - 1].endingStop;

		if (currentStop == endingStop)
		{
			for (int i = 1; i < front.size(); i++)
			{
				output << front[i-1].bus << "->" << front[i].bus << endl;
				output << getIntermediatePath(front[i-1].endingStop, front[i].endingStop, front[i].bus) << endl;
			}

			return output;
		}

		const vector<PartOfPath>& ls = mConnectionGraph[currentStop];

		for (int i = 0; i < ls.size(); i++)
		{
			const std::string& const busToNewStop = ls[i].bus;
			const std::string& const newStop = ls[i].endingStop;

			if (visitedMap.count(newStop) != 0) // this means this stop was visited as the first or last stop in a path, hence visiting it again is unecessary
				continue;

			vector<PartOfPath> newPath = front;
			newPath.emplace_back(busToNewStop, newStop);
			visitedMap[newStop] = false; // mark a stop as visited as a last or first stop in a path. (does not matter if true or false, this is used as a set)
			qu.push(std::move(newPath));
		}

		qu.pop();
	}

	throw MoovitExceptionNoRoute("");
}


void Moovit::showLineInformation() const
{
	cout << "Molimo Vas, unesite oznaku linije cije informacije zelite da prikazete." << endl;
	string busLine;
	getline(cin, busLine);

	string outputFileName = "linija_" + busLine + ".txt";
	ofstream outputFile;
	outputFile.open(outputFileName);

	if (mLineToStops.count(busLine) == 0)
	{
		cout << "Greska, trazena linija ne postoji" << busLine << endl;
		return;
	}
	if (outputFile.is_open() == false)
	{
		cout << "Greska pri obradi fajla " << outputFileName << endl;
		return;
		//throw MoovitExceptionFile(outputFileName.c_str());
	}

	ostream* outputPlace;
#ifdef P_DEBUG_FILE
	outputPlace = &cout;
#else
	outputPlace = &outputFile;
#endif

	const vector<string>& allStations = mLineToStops.at(busLine);

	(*outputPlace) << busLine << " " << mCodeToName.at(allStations[0]) << "->" << mCodeToName.at(allStations[allStations.size() - 1]) << endl;
	for (int i = 0; i < allStations.size(); i++)
		(*outputPlace) << allStations[i] << " " << mCodeToName.at(allStations[i]) << endl;
	(*outputPlace) << endl;

	cout << "Generisan je fajl " << outputFileName << " sa osnovnim informacijama o liniji " << busLine << "." << endl;
	outputFile.close();
}

void Moovit::showStopInformation() const
{

	cout << "Molimo Vas, unesite oznaku stanice cije informacije zelite da prikazete." << endl;
	string busStop;
	getline(cin, busStop);

	string outputFileName = "stajaliste_" + busStop + ".txt";
	ofstream outputFile;
	outputFile.open(outputFileName);

	if (mCodeToName.count(busStop) == 0)
	{
		cout << "Greska, nepostojeca stanica " << busStop << endl;
		return;
	}
	if (outputFile.is_open() == false)
	{
		throw MoovitExceptionFile(outputFileName.c_str());
		return;
	}

	ostream* pOutputPlace;
#ifdef P_DEBUG_FILE
	pOutputPlace = &cout;
#else
	pOutputPlace = &outputFile;
#endif

	map<std::string, bool> importantStops; // used as set
	map<std::string, bool> linesThatPassTrough; // used as set
	const vector<PartOfPath>& ref = mConnectionGraph.at(busStop);

	for (int i = 0; i < ref.size(); i++)  // this loop gathers all the lines that pass trough this stop by using a set
	{
		linesThatPassTrough[ref[i].bus] = false;
	}

	const vector<string>& directRef = mDirectConnections.at(busStop);
	for (int i = 0; i < directRef.size(); i++) // this for loop gathers all the "important" stops that are "adjacent" to this stop by using a set
	{
		if (mIsImportantCode.count(directRef[i]) != 0)
			importantStops[directRef[i]] = false;
	}

	string importantStopsString;
	string linesThatPassTroughString;
	for (auto& it : importantStops)
		importantStopsString += (std::move(it.first) + " ");
	for (auto& it : linesThatPassTrough)
		linesThatPassTroughString += (std::move(it.first) + " ");

	if(importantStopsString.size() > 0)								// this "if" and the next erase the trailing whitespace (" ") at the end of the strings
		importantStopsString.erase(importantStopsString.size() - 1); 
	if(linesThatPassTroughString.size() > 0)
		linesThatPassTroughString.erase(linesThatPassTroughString.size() - 1);
	
	(*pOutputPlace) << busStop << " " << mCodeToName.at(busStop) << " [" << linesThatPassTroughString << "] {! " << importantStopsString << " !}" << endl;

	cout << "Generisan je fajl " << outputFileName << " sa osnovnim informacijama o stanici " << busStop << "." << endl;
	outputFile.close();
}

std::string Moovit::getIntermediatePath(const std::string& startingStop, const std::string& endingStop, const std::string& bus) const
{
	// returns string for a path by a single bus line going from startingStop to endingStop

	const vector<string>& vec = mLineToStops.at(bus);

	string output;

	int startIndex = find(vec.begin(), vec.end(), startingStop) - vec.begin();
	int endIndex = find(vec.begin(), vec.end(), endingStop) - vec.begin();

	assert(startIndex + vec.begin() != vec.end());
	assert(endIndex + vec.begin() != vec.end());
	assert(startIndex != endIndex);

	if(endIndex > startIndex)
		for (int i = startIndex; i <= endIndex; i++)
			output += (vec[i] + " ");
	else
	{
		for (int i = startIndex; i >= endIndex; i--)
		{
			output += (vec[i] + " ");
		}
	}

	return output;
}

std::ostream& operator<<(std::ostream& stream, const Moovit::PartOfPath& other)
{
	stream << "bus: " << other.bus << " endingStop: " << other.endingStop;
	return stream;
}

void Moovit::readInput()
{
	readInputStops();
	readInputLines();

	cout << "Mreza gradskog prevoza je uspesno ucitana." << endl;
}

void Moovit::readInputStops()
{
	cout << "Molimo Vas, unesite putanju do fajla sa stajalistima ili kliknite ENTER za ucitavanje podrazumevanog fajla (ulazi/stajalista.txt): ";

#ifdef P_DEBUG_FILE
	mInputStopsFileName = "";
#else
	getline(cin, mInputStopsFileName);
#endif

	if (mInputStopsFileName == "")
	{
		mInputStopsFileName = "ulazi/stajalista.txt";
		mUsedDefaultStopFile = true;
	}

	ifstream inputFile;
	inputFile.open(mInputStopsFileName);

	if (inputFile.is_open())
	{
		std::string temp;

		while (getline(inputFile, temp, '\n'))
		{
			size_t separatorIndex = temp.find(" ");
			string code = temp.substr(0, separatorIndex); // code of bus stop
			string name = temp.substr(separatorIndex + 1, temp.size()); // name of bus stop

			int x = name.size();
			if(x >= 3)
				if (name[x - 1] == ']' && name[x - 2] == '!' && name[x - 3] == '[')
					mIsImportantCode.emplace(code, false); // this is only used as a "set"


			mCodeToName.insert(std::make_pair(code, std::move(name)));
		}
	}
	else
	{
		throw MoovitExceptionFile(mInputStopsFileName.c_str());
		return;
	}

	if (mUsedDefaultStopFile == true)
		cout << "Ucitan je podrazumevani fajl sa stajalistima." << endl;
	inputFile.close();
}

void Moovit::readInputLines()
{
	cout << "Molimo Vas, unesite putanju do fajla sa linijama gradskog prevoza ili kliknite ENTER za ucitavanje podrazumevanog fajla (ulazi/linije.txt): ";

#ifdef P_DEBUG_FILE
	mInputLinesFileName = "";
#else
	getline(cin, mInputLinesFileName);
#endif

	if (mInputLinesFileName == "")
	{
		mInputLinesFileName = "ulazi/linije.txt";
		mUsedDefaultLineFile = true;
	}
	ifstream inputFile;
	inputFile.open(mInputLinesFileName);

	if (inputFile.is_open() == false)
		throw MoovitExceptionFile(mInputLinesFileName.c_str());

	std::string temp;

	while (getline(inputFile, temp, '\n'))
	{
		size_t index = temp.find(" ");
		string busLine = temp.substr(0, index);

		vector<string> stops;

		size_t prevIndex = index;
		index = temp.find(' ', index+1);
		while (index != string::npos)
		{
			string stop = temp.substr(prevIndex+1, index-prevIndex-1);
			stops.push_back(std::move(stop));
			prevIndex = index;
			index = temp.find(' ', index+1);
		}
		string stop = temp.substr(prevIndex+1, index-prevIndex-1); // this and the line after this are for "flushing" the last station
		stops.push_back(std::move(stop));

		mLineToStops[busLine] = stops;

		for(int i=0; i<stops.size(); i++)
			for (int j = 0; j < i; j++)
			{
				mConnectionGraph[stops[i]].emplace_back(busLine, stops[j]);
				mConnectionGraph[stops[j]].emplace_back(busLine, stops[i]);
			}
		
		for (int i = 0; i < stops.size(); i++) // adds the stop before and after the current one to the adjecents/directs of current stop
		{
			if (i != 0)
				mDirectConnections[stops[i]].push_back(stops[i - 1]);
			if (i != stops.size() - 1)
				mDirectConnections[stops[i]].push_back(stops[i + 1]);
		}
	}

	if (mUsedDefaultLineFile == true)
		cout << "Ucitan je podrazumevani fajl sa linijama." << endl;

	inputFile.close();
}


Moovit::Moovit()
	: mUsedDefaultStopFile(false),mUsedDefaultLineFile(false)
{ }

void Moovit::doEverything()
{
	cout << "Dobrodosli u simulator mreze gradskog prevoz. ";

	string input;

	while (true)
	{
		cout << "Molimo Vas, odaberite opciju: " << endl;
		cout << "1. Ucitavanje podataka o mrezi gradskog prevoza" << endl;
		cout << "0. Kraj rada" << endl;

#ifdef P_DEBUG_FILE
		input = "1";
#else
		getline(cin, input);
#endif

		if (input == "0")
		{
			doExit();
			return;
		}
		else if (input == "1")
		{
			try
			{
				readInput();
				break;
			}
			catch (MoovitExceptionFile ex)
			{
				cout << "Greska, greska u radu sa fajlom " << ex.what() << endl;
			}
		}
		else
		{
			cout << "Greska, nevalidna opcija" << endl;
		}
	}

	while (true)
	{
		cout << "Molimo vas odaberite opciju: " << endl;
		cout << "1. Prikaz informacija o stajalistu" << endl;
		cout << "2. Prikaz informacija o liniji gradskog prevoza" << endl;
		cout << "3. Pronalazak putanje izmedju dva stajalista" << endl;
		cout << "0. Kraj rada" << endl;

		//debugAllRoutes();

		string option;
		getline(cin, option);

		if (option == "1")
			showStopInformation();
		else if (option == "2")
			showLineInformation();
		else if (option == "3")
			findRoute();
		else if (option == "0")
		{
			doExit();
			return;
		}
		else
		{
			cout << "Greska, nevalidna opcija" << endl;
			continue;
		}
	}
}

void Moovit::doExit() // only prints exit message, doesn't do destructor stuff etc...
{
	cout << "Hvala vam sto ste koristili ovaj program, sada se on gasi" << endl;
}

/*
void Moovit::debugAllRoutes()
{
	stringstream output;
	vector<string> stops = {"154", "578", "23", "1024", "103", "123", "422", "3112", "111", "1221", "99", "12", "1212", "219", "1", "312"};

	ofstream outputFile;
	outputFile.open("pantela.txt");

	for (int i=0; i<stops.size(); i++)
	{
		for (int j=i+1; j<stops.size(); j++)
		{
			outputFile << "finding start: " << stops[i] << " end: " << stops[j] << endl;
			output = findRouteLeastTransfer(stops[i], stops[j]);
			outputFile << output.rdbuf() << endl << endl;
		}
	}
	outputFile.close();
}
*/
