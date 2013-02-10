// Copyright (c) 2000 Mike Morearty <mike@morearty.com>
// Original source and docs: http://www.morearty.com/code/breakpoint

#ifndef _BREAKPOINT_H_
#define _BREAKPOINT_H_

#ifdef _DEBUG

class CBreakpoint
{
public:
	CBreakpoint() { m_index = -1; }
	~CBreakpoint() { Clear(); }

	// The enum values correspond to the values used by the Intel Pentium,
	// so don't change them!
	enum Condition { Write = 1, Read /* or write! */ = 3 };

	void Set(void* address, int len /* 1, 2, or 4 */, Condition when);
	void Clear();

protected:

	inline void SetBits(unsigned long& dw, int lowBit, int bits, int newValue)
	{
		int mask = (1 << bits) - 1; // e.g. 1 becomes 0001, 2 becomes 0011, 3 becomes 0111

		dw = (dw & ~(mask << lowBit)) | (newValue << lowBit);
	}

	int m_index; // -1 means not set; 0-3 means we've set that hardware bp
};

#endif // _DEBUG

#endif // _BREAKPOINT_H_
