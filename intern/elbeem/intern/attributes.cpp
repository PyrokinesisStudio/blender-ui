/******************************************************************************
 *
 * El'Beem - Free Surface Fluid Simulation with the Lattice Boltzmann Method
 * Copyright 2003,2004 Nils Thuerey
 *
 * configuration attribute storage class and attribute class
 *
 *****************************************************************************/

#include "attributes.h"
#include <sstream>


//! output attribute values? on=1/off=0
#define DEBUG_ATTRIBUTES 0


/******************************************************************************
 * attribute conversion functions
 *****************************************************************************/

bool Attribute::initChannel(int elemSize) {
	if(!mIsChannel) return true;
	if(mChannelInited==elemSize) {
		// already inited... ok
		return true;
	} else {
		// sanity check
		if(mChannelInited>0) {
			errMsg("Attribute::initChannel","Duplicate channel init!? ("<<mChannelInited<<" vs "<<elemSize<<")...");
			return false;
		}
	}
	
	if((mValue.size() % (elemSize+1)) !=  0) {
		errMsg("Attribute::initChannel","Invalid # elements in Attribute...");
		return false;
	}
	
	int numElems = mValue.size()/(elemSize+1);
	vector<string> newvalue;
	for(int i=0; i<numElems; i++) {
	//errMsg("ATTR"," i"<<i<<" "<<mName); // debug

		vector<string> elem(elemSize);
		for(int j=0; j<elemSize; j++) {
		//errMsg("ATTR"," j"<<j<<" "<<mValue[i*(elemSize+1)+j]  ); // debug
			elem[j] = mValue[i*(elemSize+1)+j];
		}
		mChannel.push_back(elem);
		// use first value as default
		if(i==0) newvalue = elem;
		
		double t = 0.0; // similar to getAsFloat
		const char *str = mValue[i*(elemSize+1)+elemSize].c_str();
		char *endptr;
		t = strtod(str, &endptr);
		if((str!=endptr) && (*endptr != '\0')) return false;
		mTimes.push_back(t);
		//errMsg("ATTR"," t"<<t<<" "); // debug
	}
	for(int i=0; i<numElems-1; i++) {
		if(mTimes[i]>mTimes[i+1]) {
			errMsg("Attribute::initChannel","Invalid time at entry "<<i<<" setting to "<<mTimes[i]);
			mTimes[i+1] = mTimes[i];
		}
	}

	// dont change until done with parsing, and everythings ok
	mValue = newvalue;

	mChannelInited = elemSize;
	print();
	return true;
}

// get value as string 
string Attribute::getAsString()
{
	if(mIsChannel) {
		errMsg("Attribute::getAsString", "Attribute \"" << mName << "\" used as string is a channel! Not allowed...");
		print();
		return string("");
	}
	if(mValue.size()!=1) {
		//errMsg("Attribute::getAsString", "Attribute \"" << mName << "\" used as string has invalid value '"<< getCompleteString() <<"' ");
		// for directories etc. , this might be valid! cutoff "..." first
		string comp = getCompleteString();
		if(comp.size()<2) return string("");
		return comp.substr(1, comp.size()-2);
	}
	return mValue[0];
}

// get value as integer value
int Attribute::getAsInt()
{
	bool success = true;
	int ret = 0;

	if(!initChannel(1)) success=false; 
	if(success) {
		if(mValue.size()!=1) success = false;
		else {
			const char *str = mValue[0].c_str();
			char *endptr;
			ret = strtol(str, &endptr, 10);
			if( (str==endptr) ||
					((str!=endptr) && (*endptr != '\0')) )success = false;
		}
	}

	if(!success) {
		errMsg("Attribute::getAsInt", "Attribute \"" << mName << "\" used as int has invalid value '"<< getCompleteString() <<"' ");
		errMsg("Attribute::getAsInt", "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
		errMsg("Attribute::getAsInt", "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
#if ELBEEM_PLUGIN!=1
		gElbeemState = -4; // parse error
#endif
		return 0;
	}
	return ret;
}


// get value as integer value
bool Attribute::getAsBool() 
{
	int val = getAsInt();
	if(val==0) return false;
	else 			 return true;
}


// get value as double value
double Attribute::getAsFloat()
{
	bool success = true;
	double ret = 0.0;

	if(!initChannel(1)) success=false; 
	if(success) {
		if(mValue.size()!=1) success = false;
		else {
			const char *str = mValue[0].c_str();
			char *endptr;
			ret = strtod(str, &endptr);
			if((str!=endptr) && (*endptr != '\0')) success = false;
		}
	}

	if(!success) {
		print();
		errMsg("Attribute::getAsFloat", "Attribute \"" << mName << "\" used as double has invalid value '"<< getCompleteString() <<"' ");
		errMsg("Attribute::getAsFloat", "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
		errMsg("Attribute::getAsFloat", "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
#if ELBEEM_PLUGIN!=1
		gElbeemState = -4; // parse error
#endif
		return 0.0;
	}
	return ret;
}

// get value as 3d vector 
ntlVec3d Attribute::getAsVec3d()
{
	bool success = true;
	ntlVec3d ret(0.0);

	if(!initChannel(3)) success=false; 
	if(success) {
		if(mValue.size()==1) {
			const char *str = mValue[0].c_str();
			char *endptr;
			double rval = strtod(str, &endptr);
			if( (str==endptr) ||
					((str!=endptr) && (*endptr != '\0')) )success = false;
			if(success) ret = ntlVec3d( rval );
		} else if(mValue.size()==3) {
			char *endptr;
			const char *str = NULL;

			str = mValue[0].c_str();
			double rval1 = strtod(str, &endptr);
			if( (str==endptr) ||
					((str!=endptr) && (*endptr != '\0')) )success = false;

			str = mValue[1].c_str();
			double rval2 = strtod(str, &endptr);
			if( (str==endptr) ||
					((str!=endptr) && (*endptr != '\0')) )success = false;

			str = mValue[2].c_str();
			double rval3 = strtod(str, &endptr);
			if( (str==endptr) ||
					((str!=endptr) && (*endptr != '\0')) )success = false;

			if(success) ret = ntlVec3d( rval1, rval2, rval3 );
		} else {
			success = false;
		}
	}

	if(!success) {
		errMsg("Attribute::getAsVec3d", "Attribute \"" << mName << "\" used as Vec3d has invalid value '"<< getCompleteString() <<"' ");
		errMsg("Attribute::getAsVec3d", "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
		errMsg("Attribute::getAsVec3d", "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
#if ELBEEM_PLUGIN!=1
		gElbeemState = -4; // parse error
#endif
		return ntlVec3d(0.0);
	}
	return ret;
}
		
// get value as 4x4 matrix 
ntlMat4Gfx Attribute::getAsMat4Gfx()
{
	bool success = true;
	ntlMat4Gfx ret(0.0);
	char *endptr;

	if(mValue.size()==1) {
		const char *str = mValue[0].c_str();
		double rval = strtod(str, &endptr);
		if( (str==endptr) ||
				((str!=endptr) && (*endptr != '\0')) )success = false;
		if(success) {
			ret = ntlMat4Gfx( 0.0 );
			ret.value[0][0] = rval;
			ret.value[1][1] = rval;
			ret.value[2][2] = rval;
			ret.value[3][3] = 1.0;
		}
	} else if(mValue.size()==9) {
		// 3x3
		for(int i=0; i<3;i++) {
			for(int j=0; j<3;j++) {
				const char *str = mValue[i*3+j].c_str();
				ret.value[i][j] = strtod(str, &endptr);
				if( (str==endptr) ||
						((str!=endptr) && (*endptr != '\0')) ) success = false;
			}
		}
	} else if(mValue.size()==16) {
		// 4x4
		for(int i=0; i<4;i++) {
			for(int j=0; j<4;j++) {
				const char *str = mValue[i*4+j].c_str();
				ret.value[i][j] = strtod(str, &endptr);
				if( (str==endptr) ||
						((str!=endptr) && (*endptr != '\0')) ) success = false;
			}
		}

	} else {
		success = false;
	}

	if(!success) {
		errMsg("Attribute::getAsMat4Gfx", "Attribute \"" << mName << "\" used as Mat4x4 has invalid value '"<< getCompleteString() <<"' ");
		errMsg("Attribute::getAsMat4Gfx", "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
		errMsg("Attribute::getAsMat4Gfx", "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" );
#if ELBEEM_PLUGIN!=1
		gElbeemState = -4; // parse error
#endif
		return ntlMat4Gfx(0.0);
	}
	return ret;
}
		

// get the concatenated string of all value string
string Attribute::getCompleteString()
{
	string ret;
	for(size_t i=0;i<mValue.size();i++) {
		ret += mValue[i];
		if(i<mValue.size()-1) ret += " ";
	}
	return ret;
}


/******************************************************************************
 * channel returns
 *****************************************************************************/

//! get channel as double value
AnimChannel<double> Attribute::getChannelFloat() {
	vector<double> timeVec;
	vector<double> valVec;
	
	if((!initChannel(1)) || (!mIsChannel)) {
		timeVec.push_back( 0.0 );
		valVec.push_back( getAsFloat() );
	} else {
	for(size_t i=0; i<mChannel.size(); i++) {
		mValue = mChannel[i];
		double val = getAsFloat();
		timeVec.push_back( mTimes[i] );
		valVec.push_back( val );
	}}

	return AnimChannel<double>(valVec,timeVec);
}

//! get channel as integer value
AnimChannel<int> Attribute::getChannelInt() { 
	vector<double> timeVec;
	vector<int> valVec;
	
	if((!initChannel(1)) || (!mIsChannel)) {
		timeVec.push_back( 0.0 );
		valVec.push_back( getAsInt() );
	} else {
	for(size_t i=0; i<mChannel.size(); i++) {
		mValue = mChannel[i];
		int val = getAsInt();
		timeVec.push_back( mTimes[i] );
		valVec.push_back( val );
	}}

	return AnimChannel<int>(valVec,timeVec);
}

//! get channel as integer value
AnimChannel<ntlVec3d> Attribute::getChannelVec3d() { 
	vector<double> timeVec;
	vector<ntlVec3d> valVec;
	
	if((!initChannel(3)) || (!mIsChannel)) {
		timeVec.push_back( 0.0 );
		valVec.push_back( getAsVec3d() );
	} else {
	for(size_t i=0; i<mChannel.size(); i++) {
		mValue = mChannel[i];
		ntlVec3d val = getAsVec3d();
		timeVec.push_back( mTimes[i] );
		valVec.push_back( val );
	}}

	return AnimChannel<ntlVec3d>(valVec,timeVec);
}


/******************************************************************************
 * check if there were unknown params
 *****************************************************************************/
bool AttributeList::checkUnusedParams()
{
	bool found = false;
	for(map<string, Attribute*>::iterator i=mAttrs.begin();
			i != mAttrs.end(); i++) {
		if((*i).second) {
			if(!(*i).second->getUsed()) {
				errMsg("AttributeList::checkUnusedParams", "List "<<mName<<" has unknown parameter '"<<(*i).first<<"' = '"<< mAttrs[(*i).first]->getAsString() <<"' ");
				found = true;
			}
		}
	}
	return found;
}
//! set all params to used, for invisible objects
void AttributeList::setAllUsed() {
	for(map<string, Attribute*>::iterator i=mAttrs.begin();
			i != mAttrs.end(); i++) {
		if((*i).second) {
			(*i).second->setUsed(true);
		}
	}
}

/******************************************************************************
 * Attribute list read functions
 *****************************************************************************/
int AttributeList::readInt(string name, int defaultValue, string source,string target, bool needed) {
	if(!exists(name)) {
		if(needed) { errFatal("AttributeList::readInt","Required attribute '"<<name<<"' for "<< source <<"  not set! ", SIMWORLD_INITERROR); }
		return defaultValue;
	} 
	if(DEBUG_ATTRIBUTES==1) { debugOut( source << " Var '"<< target <<"' set to '"<< find(name)->getCompleteString() <<"' as type int " , 3); }
	find(name)->setUsed(true);
	return find(name)->getAsInt(); 
}
bool AttributeList::readBool(string name, bool defaultValue, string source,string target, bool needed) {
	if(!exists(name)) {
		if(needed) { errFatal("AttributeList::readBool","Required attribute '"<<name<<"' for "<< source <<"  not set! ", SIMWORLD_INITERROR); }
		return defaultValue;
	} 
	if(DEBUG_ATTRIBUTES==1) { debugOut( source << " Var '"<< target <<"' set to '"<< find(name)->getCompleteString() <<"' as type int " , 3); }
	find(name)->setUsed(true);
	return find(name)->getAsBool(); 
}
double AttributeList::readFloat(string name, double defaultValue, string source,string target, bool needed) {
	if(!exists(name)) {
		if(needed) { errFatal("AttributeList::readFloat","Required attribute '"<<name<<"' for "<< source <<"  not set! ", SIMWORLD_INITERROR); }
		return defaultValue;
	} 
	if(DEBUG_ATTRIBUTES==1) { debugOut( source << " Var '"<< target <<"' set to '"<< find(name)->getCompleteString() <<"' as type int " , 3); }
	find(name)->setUsed(true);
	return find(name)->getAsFloat(); 
}
string AttributeList::readString(string name, string defaultValue, string source,string target, bool needed) {
	if(!exists(name)) {
		if(needed) { errFatal("AttributeList::readInt","Required attribute '"<<name<<"' for "<< source <<"  not set! ", SIMWORLD_INITERROR); }
		return defaultValue;
	} 
	if(DEBUG_ATTRIBUTES==1) { debugOut( source << " Var '"<< target <<"' set to '"<< find(name)->getCompleteString() <<"' as type int " , 3); }
	find(name)->setUsed(true);
	return find(name)->getAsString(); 
}
ntlVec3d AttributeList::readVec3d(string name, ntlVec3d defaultValue, string source,string target, bool needed) {
	if(!exists(name)) {
		if(needed) { errFatal("AttributeList::readInt","Required attribute '"<<name<<"' for "<< source <<"  not set! ", SIMWORLD_INITERROR); }
		return defaultValue;
	} 
	if(DEBUG_ATTRIBUTES==1) { debugOut( source << " Var '"<< target <<"' set to '"<< find(name)->getCompleteString() <<"' as type int " , 3); }
	find(name)->setUsed(true);
	return find(name)->getAsVec3d(); 
}

ntlMat4Gfx AttributeList::readMat4Gfx(string name, ntlMat4Gfx defaultValue, string source,string target, bool needed) {
	if(!exists(name)) {
		if(needed) { errFatal("AttributeList::readInt","Required attribute '"<<name<<"' for "<< source <<"  not set! ", SIMWORLD_INITERROR); }
		return defaultValue;
	} 
	if(DEBUG_ATTRIBUTES==1) { debugOut( source << " Var '"<< target <<"' set to '"<< find(name)->getCompleteString() <<"' as type int " , 3); }
	find(name)->setUsed(true);
	return find(name)->getAsMat4Gfx(); 
}

// set that a parameter can be given, and will be ignored...
bool AttributeList::ignoreParameter(string name, string source) {
	if(!exists(name)) return false;
	find(name)->setUsed(true);
	if(DEBUG_ATTRIBUTES==1) { debugOut( source << " Param '"<< name <<"' set but ignored... " , 3); }
	return true;
}
		
// read channels
AnimChannel<double> AttributeList::readChannelFloat(string name) {
	if(!exists(name)) { return AnimChannel<double>(0.0); } 
	return find(name)->getChannelFloat(); 
}
AnimChannel<int> AttributeList::readChannelInt(string name) {
	if(!exists(name)) { return AnimChannel<int>(0); } 
	return find(name)->getChannelInt(); 
}
AnimChannel<ntlVec3d> AttributeList::readChannelVec3d(string name) {
	if(!exists(name)) { return AnimChannel<ntlVec3d>(0.0); } 
	return find(name)->getChannelVec3d(); 
}

/******************************************************************************
 * destructor
 *****************************************************************************/
AttributeList::~AttributeList() { 
	for(map<string, Attribute*>::iterator i=mAttrs.begin();
			i != mAttrs.end(); i++) {
		if((*i).second) {
			delete (*i).second;
			(*i).second = NULL;
		}
	}
};


/******************************************************************************
 * debugging
 *****************************************************************************/

//! debug function, prints value 
void Attribute::print()
{
	std::ostringstream ostr;
	ostr << "ATTR "<< mName <<"= ";
	for(size_t i=0;i<mValue.size();i++) {
		ostr <<"'"<< mValue[i]<<"' ";
	}
	if(mIsChannel) {
		ostr << " CHANNEL: ";
		if(mChannelInited>0) {
		for(size_t i=0;i<mChannel.size();i++) {
			for(size_t j=0;j<mChannel[i].size();j++) {
				ostr <<"'"<< mChannel[i][j]<<"' ";
			}
			ostr << "@"<<mTimes[i]<<"; ";
		}
		} else {
			ostr <<" -nyi- ";
		}			
	}
	ostr <<" (at line "<<mLine<<") "; //<< std::endl;
	debugOut( ostr.str(), 10);
}
		
//! debug function, prints all attribs 
void AttributeList::print()
{
	debugOut("Attribute "<<mName<<" values:", 10);
	for(map<string, Attribute*>::iterator i=mAttrs.begin();
			i != mAttrs.end(); i++) {
		if((*i).second) {
			(*i).second->print();
		}
	}
}



/******************************************************************************
 * import attributes from other attribute list
 *****************************************************************************/
void AttributeList::import(AttributeList *oal)
{
	for(map<string, Attribute*>::iterator i=oal->mAttrs.begin();
			i !=oal->mAttrs.end(); i++) {
		// FIXME - check freeing of copyied attributes
		if((*i).second) {
			Attribute *newAttr = new Attribute( *(*i).second );
			mAttrs[ (*i).first ] = newAttr;
		}
	}
}




