#ifndef _CINFOWIFI_H_
#define _CINFOWIFI_H_

#include <iostream> // ostream

#include "ccoordinate.h"
#include "types.h" // TCID

const TCID TCID_GUEST_MIN=3;

using namespace std;

class CInfoWifi : public CCoordinate
{
		TCID Cid;

	public :

		CInfoWifi();
		CInfoWifi(TCID cid, CCoordinate coo);

		void SetCid(TCID cid);

		TCID GetCid();

		void Display(ostream& os) const;

		friend ostream& operator<<(ostream& os, const CInfoWifi& infowifi);
};

#endif
