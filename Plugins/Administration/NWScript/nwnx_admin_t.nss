#include "nwnx_admin"

void report(string func, int bSuccess)
{
    if (bSuccess)
        WriteTimestampedLogEntry("NWNX_Administration: " + func + "() success");
    else
        WriteTimestampedLogEntry("NWNX_Administration: " + func + "() failed");
}

void main()
{
    WriteTimestampedLogEntry("NWNX_Administration unit test begin..");

    string sString = "FuzzyKittens";

    NWNX_Administration_SetPlayerPassword(sString);
    report("{Set/Get}PlayerPassword", NWNX_Administration_GetPlayerPassword() == sString);

    NWNX_Administration_ClearPlayerPassword();
    report("ClearPlayerPassword", NWNX_Administration_GetPlayerPassword() == "");

    NWNX_Administration_SetDMPassword(sString);
    report("{Set/Get}DMPassword", NWNX_Administration_GetDMPassword() == sString);

    WriteTimestampedLogEntry("NWNX_Administration unit test end.");
}
