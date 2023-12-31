#pragma once
#include "Session.h"

class Player;

class GameSession : public PacketSession
{
public:
	~GameSession()
	{
		cout << "~GameSession" << endl;
	}

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

	GameSessionRef GetSessionRef() { return static_pointer_cast<GameSession> (shared_from_this()); }


	void SetMyPlayer(PlayerRef player) { _myPlayer = player; }
	void SetMyAccount(shared_ptr<class Account> account) { _myAccount = account; }
	PlayerRef GetMyPlayer() { return _myPlayer; }
	shared_ptr<class Account> GetMyAccount() { return _myAccount; }

private:
	PlayerRef _myPlayer;
	shared_ptr<class Account> _myAccount;
};