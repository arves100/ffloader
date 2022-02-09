/*!
	@author Arves100
	@file DPMsgArena.h
	@date 09/02/2022
	@brief Memory arena for System messages
*/
#pragma once

class DPMsgArena
{
public:
	DPMsgArena(size_t sz) : m_pArena((LPBYTE)malloc(sz)), m_nArenaSize(sz), m_nArenaCurr(0) {}

	~DPMsgArena() {
		free(m_pArena);
	}

	LPBYTE Store(LPVOID data, size_t sz)
	{
		if ((m_nArenaCurr + sz) > m_nArenaSize)
			m_nArenaCurr = 0; // reset arena position

		memcpy_s(m_pArena + m_nArenaCurr, m_nArenaSize - m_nArenaCurr, data, sz);
		LPBYTE r = m_pArena + m_nArenaCurr;
		m_nArenaCurr += sz;
		return r;
	}

private:
	LPBYTE m_pArena;
	size_t m_nArenaSize;
	size_t m_nArenaCurr;
};
