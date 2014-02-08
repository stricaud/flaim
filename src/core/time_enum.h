#ifndef TIME_ENUM_H_
#define TIME_ENUM_H_

#include <list>

#include "listed_record.h"
#include "core_globals.h"


class simpRecord {
public:
	uint32_t seq_no;
	uint32_t ts;
	uint32_t enumts;	
	listedRecord *rec;
		
	simpRecord(uint32_t sn, uint32_t ts, listedRecord *recArg);
	void set_enumeration(uint32_t enval);
	uint32_t getSeq() const;
	uint32_t getTS() const;
	uint32_t getEnum() const;
	int getRefCount() const;
	void decrementRefCount();	
	bool operator<(const simpRecord&);

};

class Cmp {
	
public:
	bool operator()(simpRecord sr1, simpRecord sr2) {
		return(sr1.getTS() <= sr2.getTS());
	}	
};



/////////////////////////////////////////////////////
//
//
//
//
//
//////////////////////////////////////////////////////
class BufferList {
	
	public:
		BufferList(uint32_t _baseTime, uint32_t _interval);
		clist_index_t addRecord(listedRecord* lr); // Method to add a record to the buffer. Constructs the simpRecord from a listedRecord record
		clist_index_t enumerate(); // Enumerate. calls removeTopRecords()..
		clist_index_t enumerateAll(); // Take all the elements in the buffer and enumerate them - leaving none behind. used at end.
		void setEnumeration(uint32_t, simpRecord); // Set the enumeration for a particular simple record. 
		void setCurrIndex(clist_index_t currIndex); // Set the currentIndex value.
		void setFieldName(string _fieldname); // Sets the fieldname for the bufferlist to use. 
		int size(); // Return the size of the buffer list.
		void toString(void); // prints a concise representation of the bufferList

		uint32_t baseTime, interval,enumerationIndex;
		string fieldname;
		clist_index_t lowestSeqNum; // Lowest sequence number to search for..
		clist_index_t currentIndex; // The id of the record that will be next read.
		
				
	private:
		list<simpRecord> thelist;
		int removeTopRecords(); // Method to remove the top records in the buffer (empties part of the buffer)
		clist_index_t smallestID; // Id of the smallest record that remains.. will be used next iteration.
		bool decRefCount(simpRecord* srec); // Decrement the ref count
 		bool lowestIsSet; // If true, indicates that the initial value of lowestSeqNumber has been set to the seq. no. of the first record in the bufferList.
};
#endif /*TIME_ENUM_H_*/
