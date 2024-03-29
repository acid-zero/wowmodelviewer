#ifndef DBCFILE_H
#define DBCFILE_H

#include <cassert>
#include <string>

#include <wx/wx.h>

class DBCFile
{
public:
	DBCFile(const wxString &filename);
	~DBCFile();

	// Open database. It must be openened before it can be used.
	bool open();

	// TODO: Add a close function?

	// Database exceptions
	class Exception
	{
	public:
		Exception(const wxString &message): message(message)
		{ }
		virtual ~Exception()
		{ }
		const wxString &getMessage() {return message;}
	private:
		wxString message;
	};

	// 
	class NotFound: public Exception
	{
	public:
		NotFound(): Exception(wxT("Key was not found"))
		{ }
	};

	// Iteration over database
	class Iterator;
	class Record
	{
	public:
		Record& operator= (const Record& r)
		{
            file = r.file;
			offset = r.offset;
			return *this;
		}
		float getFloat(size_t field) const
		{
			assert(field < file.fieldCount);
			return *reinterpret_cast<float*>(offset+field*4);
		}
		unsigned int getUInt(size_t field) const
		{
			assert(field < file.fieldCount);
			return *reinterpret_cast<unsigned int*>(offset+(field*4));
		}
		int getInt(size_t field) const
		{
			assert(field < file.fieldCount);
			return *reinterpret_cast<int*>(offset+field*4);
		}
		unsigned char getByte(size_t ofs) const
		{
			assert(ofs < file.recordSize);
			return *reinterpret_cast<unsigned char*>(offset+ofs);
		}
		wxString getString(size_t field) const
		{
			assert(field < file.fieldCount);
			size_t stringOffset = getUInt(field);
			if (stringOffset >= file.stringSize)
				stringOffset = 0;
			assert(stringOffset < file.stringSize);
			//char * tmp = (char*)file.stringTable + stringOffset;
			//unsigned char * tmp2 = file.stringTable + stringOffset;
			return wxString(reinterpret_cast<char*>(file.stringTable + stringOffset), wxConvUTF8);
		}
	private:
		DBCFile &file;
		unsigned char *offset;
		Record(DBCFile &file, unsigned char *offset): file(file), offset(offset) {}

		friend class DBCFile;
		friend class Iterator;
	};

	/* Iterator that iterates over records */
	class Iterator
	{
	public:
		Iterator(DBCFile &file, unsigned char *offset): 
			record(file, offset) {}
		/// Advance (prefix only)
		Iterator & operator++() { 
			record.offset += record.file.recordSize;
			return *this; 
		}	
		/// Return address of current instance
		Record const & operator*() const { return record; }
		const Record* operator->() const {
			return &record;
		}
		/// Comparison
		bool operator==(const Iterator &b) const
		{
			return record.offset == b.record.offset;
		}
		bool operator!=(const Iterator &b) const
		{
			return record.offset != b.record.offset;
		}
	private:
		Record record;
	};

	// Get record by id
	Record getRecord(size_t id);
	/// Get begin iterator over records
	Iterator begin();
	/// Get begin iterator over records
	Iterator end();
	/// Trivial
	size_t getRecordCount() const { return recordCount; }
	size_t getFieldCount() const { return fieldCount; }
	wxString getFilename() { return filename; }
	size_t size() const { return recordCount; }

private:
	wxString filename;
	size_t recordSize;
	size_t recordCount;
	size_t fieldCount;
	size_t stringSize;
	unsigned char *data;
	unsigned char *stringTable;
};

#endif
