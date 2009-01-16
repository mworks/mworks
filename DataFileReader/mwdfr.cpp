#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "Scarab/scarab.h"

// global to store the codec
ScarabDatum *codec;

long long getScarabEventTime(ScarabDatum *datum);
unsigned int getScarabEventCode(ScarabDatum *datum);
ScarabDatum *getScarabEventPayload(ScarabDatum *datum);

ScarabSession *connectToFile(const std::string &file);

double scarab_extract_float(ScarabDatum *datum);

// used so that the tool doesn't write out huge quantities of crap binary data
bool isString(const char *tmp, int length);

std::string line();
std::string scarabEventToString(const int index, 
								ScarabDatum *datum);
std::string niceNumber(const unsigned long num);
std::string number(const long long num);
std::string getScarabEventData(ScarabDatum *datum);
std::string recursiveGetScarabDict(ScarabDatum *datum);
std::string recursiveGetScarabList(ScarabDatum *datum);
std::string scarabValue(ScarabDatum *payload);
std::string nameForCode(const int code);
std::map<std::string, int> getVariables(const std::string &file);

int main( int argc, char *argv[])
{
	using namespace std;
	
	codec = 0;
	
	if ( argc != 2 ) { 
		cout << "usage: " << argv[0] << " filename" << endl;
		return -1;
	}
	ifstream the_file ( argv[1] );
	if ( !the_file.is_open() ) {
		cout << "Could not open " << argv[1] << endl;
		return -2;
	}
	
	ScarabSession *session = connectToFile(argv[1]);
	ScarabDatum *datum;
	int count = 0;
	while(datum = scarab_read(session)){
		count++;
		// All events should be scarab lists
		if(datum->type != SCARAB_LIST){  
			cout << "Invalid event structure..." << endl;
			break;
		}
		
		cout << scarabEventToString(count, datum);
		
		scarab_free_datum(datum);
	}
	
	scarab_session_close(session);
	the_file.close();
}

std::string line() {
	std::string ret;
	for(int i = 0; i<80; ++i) {
		ret += "#";
	}
	return ret + "\n";
}

std::string scarabEventToString(const int event_number, 
								ScarabDatum *datum){
	using namespace std;
	string ret("");
	
	unsigned long code = getScarabEventCode(datum);
	
	
	if(code == 0) {
		if(codec) {
			scarab_free_datum(codec);
		}
		codec = scarab_copy_datum(datum);
	}
	
	long long time = getScarabEventTime(datum);
	
	ret += niceNumber(event_number) + 
    ":" + nameForCode(code) + 
    "(" + number(code) + 
    ") :" + number(time) + "\n  ";
	
	ret += getScarabEventData(datum);
	ret += "\n";
	ret += line();
	
	return ret;
}

double scarab_extract_float(ScarabDatum *datum){
#if __LITTLE_ENDIAN__
	return *((double *)(datum->data.opaque.data));
#else
	int i;
	unsigned char swap_buffer[sizeof(double)];
	unsigned char *datum_bytes = (unsigned char *)datum->data.opaque.data;
	for(i = 0; i < sizeof(double); i++){
		swap_buffer[i] = datum_bytes[sizeof(double) - i - 1];
	}
	
	return *((double *)swap_buffer);
#endif
}

std::string nameForCode(const int code) {
	if(codec == 0 || code == 0 || code == 1 || code == 2) {
		return std::string("");
	}
	
	ScarabDatum *key = scarab_new_integer(code);
	ScarabDatum *tag = scarab_new_string("tagname");
	const int SCARAB_SYSTEM_EVENT_PAYLOAD_INDEX=2;
	
	std::string ret((const char *)(scarab_dict_get(scarab_dict_get(scarab_list_get(codec, SCARAB_SYSTEM_EVENT_PAYLOAD_INDEX),key), tag))->data.opaque.data);
	scarab_free_datum(tag);
	scarab_free_datum(key);
	
	return ret;
}

ScarabSession *connectToFile(const std::string &file) {
	using namespace std;
	
	string uri = "ldobinary:file://" + file;
	
//	cerr << "opening uri: " << uri << endl;
	scarab_init();
	
	
	// I hate myself for this
	char *stupid_crap = new char[uri.length() + 1];
	strncpy(stupid_crap, uri.c_str(), uri.length() + 1);
	return scarab_session_connect(stupid_crap);
}


std::string number(const long long num) {
	std::string ret;
	std::stringstream stream;
	stream << num;
	
	ret = stream.str();
	return ret;
}

std::string niceNumber(const unsigned long num) {
	std::string ret;
	std::stringstream stream;
	stream << num;
	
	ret = stream.str();
	
	switch(ret.length()) {
		case 1:
			ret.append("             ");
			break;
		case 2:
			ret.append("            ");
			break;
		case 3:
			ret.append("           ");
			break;
		case 4:
			ret.append("           ");
			break;
		case 5:
			ret.append("          ");
			break;
		case 6:
			ret.append("         ");
			break;
		case 7:
			ret.append("        ");
			break;
		case 8:
			ret.append("       ");
			break;
		case 9:
			ret.append("      ");
			break;
		case 10:
			ret.append("     ");
			break;
		case 11:
			ret.append("    ");
			break;
		case 12:
			ret.append("   ");
			break;
		case 13:
			ret.append("  ");
			break;
		case 14:
			ret.append(" ");
			break;
		case 15:
			ret.append("");
			break;
		default:
			ret.append("OUCH");
			break;
	}
	
	return ret;
}

unsigned int getScarabEventCode(ScarabDatum *datum){
	const int SCARAB_EVENT_CODEC_CODE_INDEX = 0;
	ScarabDatum *code_datum = scarab_list_get(datum, SCARAB_EVENT_CODEC_CODE_INDEX);
	return code_datum->data.integer;
	
}

long long getScarabEventTime(ScarabDatum *datum){
	const int SCARAB_EVENT_TIME_INDEX = 1;
	ScarabDatum *time_datum = scarab_list_get(datum, SCARAB_EVENT_TIME_INDEX);
	return time_datum->data.integer;
}


ScarabDatum *getScarabEventPayload(ScarabDatum *datum){
	const int SCARAB_EVENT_PAYLOAD_INDEX = 2;
	ScarabDatum *payload_datum = scarab_list_get(datum, SCARAB_EVENT_PAYLOAD_INDEX);
	return payload_datum;
}


std::string getScarabEventData(ScarabDatum *datum) {
	using namespace std;
	
	if(datum == NULL){
		return string("Bad data event");
	}
	
	ScarabDatum *payload = getScarabEventPayload(datum);
	
	if(!payload){
		return string("Bad data event");
	}
	
	return scarabValue(payload);
}

std::string scarabValue(ScarabDatum *payload) {
	using namespace std;
	
	if(payload->type == SCARAB_INTEGER){
		string ret("INT: ");
		std::stringstream stream;
		stream << payload->data.integer;
		
		return ret + stream.str();
	} else if(payload->type == SCARAB_FLOAT){
		string ret("DOUBLE: ");
		std::stringstream stream;
		stream << (double)payload->data.floatp;
		
		return ret + stream.str();
	} else if(payload->type == SCARAB_FLOAT_NAN ||
			  payload->type == SCARAB_FLOAT_OPAQUE) {
		string ret("DOUBLE: ");
		std::stringstream stream;
		stream << scarab_extract_float(payload);
		
		return ret + stream.str();
	} else if(payload->type == SCARAB_OPAQUE){
		string ret;
		char *tmp = scarab_extract_string(payload);
		if(isString(tmp, payload->data.opaque.size)) {
			ret = ret + "STRING: " + tmp;
		} else {
			ret += "OPAQUE";
		}
		delete [] tmp;
		return ret;
	} else if(payload->type == SCARAB_DICT){
		return recursiveGetScarabDict(payload);
	} else if(payload->type == SCARAB_LIST){
		return recursiveGetScarabList(payload);
	}
	
	return string("NULL");
}

bool isString(const char *tmp, int length) {
	for(int i=0; i<length-1; ++i) {
		if(tmp[i] < 32 || tmp[i] > 126) {
			return false;
		}
	}
	
	return true;
}

std::string recursiveGetScarabDict(ScarabDatum *datum) {
	using namespace std;
	
	string ret("DICT:");
	if(datum->type != SCARAB_DICT){
		return ret + " ERROR";
	}
	
	int n = scarab_dict_number_of_elements(datum);
	ScarabDatum **keys = scarab_dict_keys(datum);
	ScarabDatum **values = scarab_dict_values(datum);
	
	for(int i = 0; i < n; i++){
		
		ret = ret + "\n   KEY: " + scarabValue(keys[i]) + "  VALUE: ";
		if(values[i] != NULL){
			std::stringstream stream;
			
			switch(values[i]->type){
					
				case SCARAB_INTEGER:
					stream << values[i]->data.integer;
					ret += "(int)" + stream.str();
					break;
				case SCARAB_FLOAT:
					stream << values[i]->data.floatp;
					ret += "(double)" + stream.str();
					break;
				case SCARAB_FLOAT_OPAQUE:
					stream << scarab_extract_float(values[i]);
					ret += "(double)" + stream.str();
					break;
				case SCARAB_OPAQUE: 
				{
					char *tmp = scarab_extract_string(values[i]);
					if(isString(tmp, values[i]->data.opaque.size)) {
						ret = ret + "(string)" + tmp;
					} else {
						ret += "(binary)";
					}
					delete [] tmp;
				}
					break;
				case SCARAB_DICT:
					ret += recursiveGetScarabDict(values[i]);
					break;
				case SCARAB_LIST:
					ret += recursiveGetScarabList(values[i]);
					break;
				default:
					ret += "(NULL)";
					break;
			}
		}      
	}
	
	return ret;
}

std::string recursiveGetScarabList(ScarabDatum *datum){
	using namespace std;
	
	string ret("LIST:");
	if(datum->type != SCARAB_LIST){
		return ret + " ERROR";
	}
	
	int n = datum->data.list->size;
	ScarabDatum **values = datum->data.list->values;
	
	
	for(int i = 0; i < n; i++){
		stringstream index;
		index << i;
		ret = ret + "\n  INDEX: " + index.str();
		
		stringstream num;
		if(values[i] != NULL){
			
			switch(values[i]->type){
					
				case SCARAB_INTEGER:
					num << values[i]->data.integer;
					ret += "(int)" + num.str();
					break;
				case SCARAB_FLOAT:
					num << values[i]->data.floatp;
					ret += "(double)" + num.str();
					break;
				case SCARAB_FLOAT_OPAQUE:
					num << scarab_extract_float(values[i]);
					ret += "(double)" + num.str();
					break;
				case SCARAB_OPAQUE: 
				{
					char *tmp = scarab_extract_string(values[i]);
					if(isString(tmp, values[i]->data.opaque.size)) {
						ret = ret + "(string)" + tmp;
					} else {
						ret += "(binary)";
					}
					delete [] tmp;
				}
					break;
				case SCARAB_DICT:
					ret += recursiveGetScarabDict(values[i]);
					break;
				case SCARAB_LIST:
					ret += recursiveGetScarabList(values[i]);
					break;
				default:
					ret += "(NULL)";
					break;
			}
		}
	}
	
	return ret;
}

bool useEvent(const std::vector<std::string> &eventsToInclude, 
			  const std::string &eventName) {
	for(std::vector<std::string>::const_iterator i = eventsToInclude.begin();
		i != eventsToInclude.end();
		++i) {
		if(*i == eventName) {
			return true;
		}
	}
	
	return false;
}

std::map<std::string, int> getVariables(const std::string &file) {
	std::map<std::string, int> retval;
	
	ScarabSession *session = connectToFile(file);
	
	ScarabDatum *datum = 0;
	while(datum = scarab_read(session)) {
		if(datum->type != SCARAB_LIST){
			std::cerr << "Invalid event structure.  Quitting." << std::endl;
			scarab_free_datum(datum);
			exit(-1);
		}
		
		if(getScarabEventCode(datum) == 1) {
			
			ScarabDatum *codec = getScarabEventPayload(datum);
			int n_codec_entries = codec->data.dict->tablesize;
			ScarabDatum **keys = scarab_dict_keys(codec);
			
			for(int c = 0; c < n_codec_entries; ++c){
				if(keys[c]) {
					int code = keys[c]->data.integer;
					ScarabDatum *serializedVar = scarab_dict_get(codec, keys[c]);
					
					if(serializedVar && serializedVar->type == SCARAB_DICT) {
						ScarabDatum *name_key = scarab_new_string("tagname");
						ScarabDatum *tagnameDatum = scarab_dict_get(serializedVar, name_key);
						scarab_free_datum(name_key);
						
						if(tagnameDatum) {
							std::string tagname(scarab_extract_string(tagnameDatum));
							retval[tagname] = code;
						} else {
							std::cerr << "Variable doesn't have a tagname.  Exiting." << std::endl;
						}
					}
				}
			}
			scarab_free_datum(datum);
			break;
		}
		
		scarab_free_datum(datum);
	}
	
	scarab_session_close(session);
	
	return retval;
}
