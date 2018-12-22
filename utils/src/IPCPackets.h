#pragma once

namespace simplefs
{
	typedef struct
	{
		int shmid;
		unsigned int offset;
		unsigned int size;
	} ShmemPtr;

	class Packet
	{
	public:
		static Packet* fromId(int id);


		virtual int getBaseLength() = 0;
		virtual int getRemainderLength() { return 0; } //for packets with varying size

		int getTotalLength();

		virtual void deserializeBase(const char* data) = 0;
		virtual char* getRemainderBuffer() { return nullptr; } //for packets with varying size

		virtual void serialize(char* data) = 0;
	};

	class OperationWithPathRequest : public Packet
	{
	public:
		typedef enum {
			Create = 'CREA',
			Open = 'OPEN',
			Chmd = 'CHMD',
			Unlink = 'ULNK',
			Mkdir = 'MKDR'
		} OpType;

		OperationWithPathRequest(OpType type) : type(type) {}
		~OperationWithPathRequest();

		virtual int getBaseLength();
		virtual int getRemainderLength();

		virtual void deserializeBase(const char* data);
		virtual char* getRemainderBuffer();

		virtual void serialize(char* data);

		int getMode();
		void setMode(int);
		
		const char* getPath();
		void setPath(const char*);

		OpType getType();
	private:
		OperationWithPathRequest(OperationWithPathRequest&);
		OperationWithPathRequest(OperationWithPathRequest&&);
		OperationWithPathRequest& operator=(OperationWithPathRequest);

		int mode;
		char* path = nullptr;
		int pathLen = 0;
		OpType type;
	};

	class LSeekRequest : public Packet
	{
	public:
		static const unsigned int ID = 'SEEK';
		
		virtual int getBaseLength();

		virtual void deserializeBase(const char* data);

		virtual void serialize(char* data);

		int getFD();
		void setFD(int);

		int getOffset();
		void setOffset(int);

		int getWhence();
		void setWhence(int);

	private:
		int fd;
		int offset;
		int whence;
	};

	class ReadRequest : public Packet
	{
	public:
		static const unsigned int ID = 'READ';
		
		virtual int getBaseLength();

		virtual void deserializeBase(const char* data);

		virtual void serialize(char* data);

		int getFD();
		void setFD(int);

	private:
		int fd;
	};

	class WriteRequest : public Packet
	{
	public:
		static const unsigned int ID = 'WRTE';
		
		virtual int getBaseLength();

		virtual void deserializeBase(const char* data);

		virtual void serialize(char* data);

		int getFD();
		void setFD(int);

		int getLen();
		void setLen(int);

	private:
		int fd;
		int len;
	};

	class CloseRequest : public Packet
	{
	public:
		static const unsigned int ID = 'CLSE';
		
		virtual int getBaseLength();

		virtual void deserializeBase(const char* data);

		virtual void serialize(char* data);

		int getFD();
		void setFD(int);

	private:
		int fd;
	};

	class OKResponse : public Packet
	{
	public:
		static const unsigned int ID = 'OKOK';
		
		virtual int getBaseLength();

		virtual void deserializeBase(const char* data);

		virtual void serialize(char* data);

	};

	class ErrorResponse : public Packet
	{
	public:
		static const unsigned int ID = 'FAIL';
		
		virtual int getBaseLength();

		virtual void deserializeBase(const char* data);

		virtual void serialize(char* data);

		int getErrno();
		void setErrno(int);
	
	private:
		int errno;
	};

	class ShmemPtrResponse : public Packet
	{
	public:
		static const unsigned int ID = 'SHMP';
		
		virtual int getBaseLength();

		virtual void deserializeBase(const char* data);

		virtual void serialize(char* data);

		ShmemPtr getPtr();
		void setPtr(ShmemPtr);
	private:
		ShmemPtr ptr;
	};
}


