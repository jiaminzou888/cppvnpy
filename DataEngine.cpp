#include "DataEngine.h"

bool DataEngine::de_get_contract(QString vtSymbol, InstrumentInfo& contract)
{
	if (allInstruments.find(vtSymbol) != allInstruments.end())
	{
		contract = allInstruments[vtSymbol];
		return true;
	}
	else
	{
		return false;
	}
}

bool DataEngine::de_get_order(QString ordID, OrderInfo& ordInfo)
{
	if (allOrderDict.find(ordID) != allOrderDict.end())
	{
		ordInfo = allOrderDict[ordID];
		return true;
	}
	else
	{
		return false;
	}
}