#include "time_enum.h"

simpRecord::simpRecord(uint32_t sn, uint32_t v, listedRecord *recArg) {
	seq_no = sn;
	ts = v;
	rec = recArg;
}
	
uint32_t simpRecord::getSeq() const {
	return seq_no;
}

uint32_t simpRecord::getTS() const {
	return ts;	
}


uint32_t simpRecord::getEnum() const {
	return enumts;	
}

int simpRecord::getRefCount() const {
	return(rec->refCount);
}

void simpRecord::decrementRefCount(){
	rec->refCount = getRefCount()-1;
//	return(rec->refCount);
}

bool simpRecord::operator<( const simpRecord& s) {
	return(ts<s.getTS());
	
}

void simpRecord::set_enumeration(uint32_t enval) {
	enumts = enval;	
}

///////////////////////////
//
////////////////////////////
BufferList::BufferList(uint32_t _baseTime, uint32_t _interval) {
	baseTime = _baseTime;
	interval= _interval;
//	fieldname = _fieldname;
	smallestID = 0;
	// This is not correct - if first record does not have field then lowestSeqNum should be set to the first seq. no. that does.
	// This is changed via the lowestIsSet flag and the addRecord() Method..
	lowestSeqNum = 0; 
	
	enumerationIndex = baseTime;
	lowestIsSet = false;
	if (verbose)
		cerr << "BufferList- Base time: "<<baseTime<<" interval: "<<interval<<" fieldname: "<<fieldname<<" enumIndex: "<<enumerationIndex<<endl;

}
///////////////////////////////////
//
////////////////////////////////////
void BufferList::setFieldName(string _fieldname) {
	fieldname = _fieldname;	
}
///////////////////////////////////
//
//////////////////////////////////////
int BufferList::size() {
	
	return(thelist.size());
}

////////////////////////////
////
////////////////////////////
void BufferList::toString(void) {
	list<simpRecord>::iterator recIter = thelist.begin();
	for (recIter;recIter != thelist.end();++recIter) {
		cerr << "("<<recIter->getSeq()<<","<<recIter->getTS()<<","<<recIter->getEnum()<<","<<recIter->getRefCount()<<")-";
	}
	cerr << endl;
}


//////////////////////////////////////////////////
// Return the id of the record just addedd. If the record did not
// have the needed field, return -1*id. 
// This method adds a record to the Buffer list if it has the
// needed fieldname. Otherwise it just decrements the 
// refcount of the records..
//////////////////////////////////////////////////
clist_index_t BufferList::addRecord(listedRecord* lr) {

	// Check that the needed field is present. 
	// If field present add to buffer list.
	// If field is NOT present, decrement ref counter and proceed. 
	// Increment current id.

	Value val;
	// Construct a simpRecord from the listedRecord

	// Get the field we want to anonymize. 
	// If it doesn't exist in this record, getField will return a 1.
	int ret = lr->theRecord->getField((char*)fieldname.c_str(),val);
	
	if (ret==0) {
		// success!!
		if (verbose)
			val.dumpPrint(" Field of record: ");
		
		// Add the simpRecord to thelist
		simpRecord sr(lr->id,*Value::to_uint32(&val), lr);
		thelist.push_back(sr);

		if (verbose) 
			cerr << "Added record with seqnum: "<<lr->id<<" to the buffer list. ts: "<<*Value::to_uint32(&val)<<" lr: "<<lr<<endl;

		// This means that this record is the first to be added to the buffer list. Therefore
		// set the lowestSeqNum to this records sequence number.
		if (!lowestIsSet) {
			lowestSeqNum = lr->id;
			lowestIsSet = true;
		}
		return(lr->id);
			
	} else {
		// Failure!
//		 Decrement the refcount..	
		--(lr->refCount);		
		if (verbose) {
			cerr << "Failed to add record with seqnum: "<<lr->id<<" to buffer because did not have field: "<<fieldname<<endl;			
		}
		return(-1*(lr->id));
	}
}
//////////////////////////////////////////////////
// Return the id of the record just addedd.
//////////////////////////////////////////////////
//clist_index_t BufferList::addRecord(listedRecord* lr) {
//	
//	Value val;
//	// Construct a simpRecord from the listedRecord
//
//	// Get the timestamp value..
//	lr->theRecord->getField((char*)fieldname.c_str(),val);
//	if (verbose)
//		val.dumpPrint(" nothing..");
//	// Add the simpRecord to thelist
//	simpRecord sr(lr->id,*Value::to_uint32(&val), lr);
//	thelist.push_back(sr);
//
//	if (verbose) 
//		cerr << "Added record with seqnum: "<<lr->id<<" to the buffer list. ts: "<<*Value::to_uint32(&val)<<" lr: "<<lr<<endl;
//			
//	return(lr->id);
//}

///////////////////////////////////////////////////////////////////////
// Return the id of the least record that hasn't been enumerated yet.
// this particular record will be in the next iteration.
///////////////////////////////////////////////////////////////////
clist_index_t BufferList::enumerate() {
	
	// sort and enumerate the records
	
	thelist.sort(Cmp());
	if (verbose) {
//		cerr << "After sorting: "<<endl;
//		toString();
	}
	
	// Get an iterator for the bufferlist
	typedef list<simpRecord>::iterator lsr;
	lsr recIter = thelist.begin();
	clist_index_t nextLowestSeqNum;

	if (lowestSeqNum == std::numeric_limits<uint32_t>::max()) {
		// This means that there weren't any records remaining in the buffer
		// at the end of the last iteration. Now we just set lowestSeqNum to 
		// the value of the first element in the buff list.
		lowestSeqNum = thelist.front().getSeq();
		if (verbose) 
			cerr << "BufferList: lowest index changed to: "<<lowestSeqNum<<endl;
			
	}
	
	if (verbose) 
		cerr << "BufferList: lowest index: "<<lowestSeqNum<<endl;
	// Loop until we have no more records..
	while (recIter != thelist.end()) {
		simpRecord temp_rec = (*recIter);
		clist_index_t tempSeq = temp_rec.getSeq();
		
//		 Now enumerate the element..
		if (verbose) 
			cerr << "BufferList: setting record "<<tempSeq<<" value to: "<<enumerationIndex<<endl;
			
		setEnumeration(enumerationIndex,*recIter);
		enumerationIndex+=interval;



		// If the current record is the one we are searching for..
		if (tempSeq == lowestSeqNum) {
			// .. then we can rejoice! Now we must remove all the records before it, and 
			// find the lowestSeqNum in the remaining records.
			lsr savedRecIter(recIter);
			++recIter;
			// use the maximum value of uint32_t
			nextLowestSeqNum = std::numeric_limits<uint32_t>::max(); //currentIndex;
			// Now loop through the rest of the elements, trying to find the lowest one..
			// We want to keep track of the lowest sequence number in the new buffer..
			while (recIter != thelist.end()) {
				if ((*recIter).getSeq() < nextLowestSeqNum) {
					nextLowestSeqNum = (*recIter).getSeq();
				}
				++recIter;
			}
			
			// Set the sequence number correctly..
			lowestSeqNum = nextLowestSeqNum;
			if (verbose) 
				cerr << "BufferList: in remove: Next lowest sequence number: "<<lowestSeqNum<<endl;
				 
			// Now we will iterate through all the records we have correctly enumerated and
			// decrement the refCount on them and remove them from the list.
			lsr newRecIter = thelist.begin();
			++savedRecIter;
			
			for (;newRecIter != savedRecIter;++newRecIter) {
				decRefCount(&(*newRecIter));
			}
			// Remove the elements.
			thelist.erase(thelist.begin(),savedRecIter);	
			return(lowestSeqNum);		
		}
		++recIter;

	}
	if (verbose) 
		cerr << " BufferList: We shouldn't be getting here!!!"<<endl;
}
////////////////////////////////////////////////////////////////////
// Decrement the refCount of an element in the bufferlist
// index is the index of the element in the bufferList
//////////////////////////////////////////////////////////////////////////
bool BufferList::decRefCount(simpRecord* srec) {

	// Get the sequence number
	clist_index_t id = srec->getSeq();
	int oldrefcount = srec->getRefCount();	
	
	// Decrement the ref count..
	srec->decrementRefCount();

	if (verbose) 
		cerr <<"BufferList: Decrementing refcount of "<<id<<" from: "<<oldrefcount<<" to: "<<srec->getRefCount()<<" \n";
	
}

/////////////////////////////////////////////////////
// Sets the enumeration of a record..
//////////////////////////////////////////////////////
void BufferList::setEnumeration(uint32_t newTimeStamp, simpRecord srec) {
	
	Value val, val1;
	//  Get the listedRecord from the simp record.
	listedRecord* lr = srec.rec;
	
	// Get the timestamp value..
	lr->theRecord->getField((char*)fieldname.c_str(),val);
	
	uint32_t *z2 = Value::to_uint32(&val);
	if (verbose) 
		cerr << "-- Previous value of ts: "<<*z2;
        
    *z2 = newTimeStamp;
    
    
    lr->theRecord->getField((char*)fieldname.c_str(),val1);
    if (verbose)
    	cerr << " new value: "<<*Value::to_uint32(&val1)<<endl;
	
}
///////////////////////////////////////////////////////////////////////
// Enumerate all the remaining records.
// Return the id of the last record enumerated.
///////////////////////////////////////////////////////////////////
clist_index_t BufferList::enumerateAll() {

	// sort and enumerate the records
	
	thelist.sort(Cmp());
	if (verbose) {
//		cout << "After sorting: "<<endl;
//		toString();
	}
	
	// Get an iterator for the bufferlist
	typedef list<simpRecord>::iterator lsr;
	lsr recIter = thelist.begin();
	clist_index_t nextLowestSeqNum;

	if (verbose) 
		cerr << "BufferList: lowest index: "<<lowestSeqNum<<endl;


	// Enumerate all is simpler than enumerate, all we do is go through the entire list
	// after sorting and enumerate each one. Then decrement the refCount for each one.	
	while (recIter != thelist.end()) {
		simpRecord temp_rec = (*recIter);
		clist_index_t tempSeq = temp_rec.getSeq();
		if (verbose) 
			cerr << "BufferList: in remove: processing seqnum: "<<tempSeq<<endl;
		
		decRefCount(&(*recIter));

		// Now enumerate the element..
		if (verbose) 
			cerr << "BufferList: setting record "<<tempSeq<<" value to: "<<enumerationIndex<<endl;
		setEnumeration(enumerationIndex,*recIter);
		enumerationIndex+=interval;

		++recIter;
	}
	// Remove the elements.
	thelist.erase(thelist.begin(),thelist.end());	

}
void BufferList::setCurrIndex(clist_index_t val) {
	currentIndex = val;
	
}
/////////////////////////////////////////
//
/////////////////////////////////////////
int BufferList::removeTopRecords() {
	// Decide which simpRecords from thelist i should remove
	// figure out the smallest sequence number in the list.
		
}
