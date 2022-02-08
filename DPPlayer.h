/*!
	@author Arves100
	@brief DirectPlay player rapresentation
	@date 07/02/2022
	@file DPPlayer.h
*/
#pragma once

class DPPlayer
{
public:
	DPPlayer();
	~DPPlayer();

	void ForceDisconnect();
	void Disconnect();

	ENetPeer* GetPeer() const { return m_pPeer; }
	bool IsHostMade() const { return m_bMadeByHost; }
	DPID GetId() const { return m_dwId; }
	LPVOID GetLocalData() const { return m_lpData; }
	DWORD GetLocalDataSize() const { return m_dwDataSize; }
	LPVOID GetRemoteData() const { return m_lpRemoteData; }
	DWORD GetRemoteDataSize() const { return m_dwRemoteDataSize; }
	bool IsSpecator() const { return m_bIsSpectator; }
	bool IsMadeByHost() const { return m_bMadeByHost; }
	const char* GetLongName() const { return m_szLongName.c_str(); }
	const char* GetShortName() const { return m_szShortName.c_str(); }

	void FireEvent();
	void Create(DPID id, const char* shortName, const char* longName, HANDLE hEvent, LPVOID lpData, DWORD dwDataSize, bool spectator, bool madeByHost);

private:
	DPID m_dwId;
	std::string m_szLongName;
	std::string m_szShortName;
	HANDLE m_hEvent;
	LPVOID m_lpData;
	DWORD m_dwDataSize;
	bool m_bIsSpectator;
	bool m_bMadeByHost;
	LPVOID m_lpRemoteData;
	DWORD m_dwRemoteDataSize;

	// ENet specific
	ENetPeer* m_pPeer;
};
