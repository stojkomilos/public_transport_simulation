#pragma once

#include<assert.h>
#include<string>
#include<map>
#include<vector>
#include<tuple>
#include<sstream>

// used for chaning the program so it's more easy to debug
#define P_DEBUG_FILE 1

class Moovit // a singleton class
{
public:
	Moovit(const Moovit&) = delete;
	Moovit operator=(const Moovit&) = delete;

	static Moovit& get()
	{
		static Moovit instance;
		return instance;
	}

	void doEverything();

private:
	struct PartOfPath
	{
		std::string bus;
		std::string endingStop;
		int distance;

		PartOfPath(const std::string& bus, const std::string& endingStop)
			: bus(bus), endingStop(endingStop)
		{ }
	};

	Moovit();
	
	std::map<std::string, std::string> mCodeToName;
	std::map<std::string, std::vector<std::string>> mLineToStops;
	std::map<std::string, std::vector<PartOfPath>> mConnectionGraph;
	std::map<std::string, bool> mIsImportantCode; // a set

	std::map<std::string, std::vector<std::string>> mDirectConnections;

	std::string mInputLinesFileName;
	std::string mInputStopsFileName;

	bool mUsedDefaultStopFile;
	bool mUsedDefaultLineFile;

	void doExit();

	void readInput();
	void readInputLines();
	void readInputStops();

	void showStopInformation() const;
	void showLineInformation() const;
	void findRoute();

	std::stringstream findRouteImportant(const std::string& startingStop, const std::string& endingStop);
	std::stringstream findRouteLeastTransfer(const std::string& startingStop, const std::string& endingStop);
	std::tuple<std::string, std::string> findRouteGeneralPre();
	void findRouteGeneralPost(const std::string& startingStop, const std::string& endingStop, const std::stringstream& output);
	std::string getIntermediatePath(const std::string& startingStop, const std::string& endingStop, const std::string& bus) const;

	friend std::ostream& operator<<(std::ostream& stream, const PartOfPath& other);

	void debugAllRoutes();
};


std::ostream& operator<<(std::ostream& stream, const Moovit::PartOfPath& other);
	
