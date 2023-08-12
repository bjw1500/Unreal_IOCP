#include "pch.h"
#include "Monster.h"
#include "GameRoom.h"
#include "Player.h"
#include "ServerPacketHandler.h"
#include "DataManager.h"
#include "DataContents.h"
#include <format>
#include "Protocol.pb.h"
#include <random>

Monster::Monster() : Creature()
{
}

Monster::~Monster()
{
}

void Monster::Update(float deltaTime)
{
	UpdateController();



}

void Monster::UpdateController()
{
	switch (GetState())
	{
	case Idle:
		UpdateIdle();
		break;
	case Move:
		UpdateMove();
		break;
	case Attack:
		UpdateAttack();
		break;
	case Dead:
		UpdateDead();
		break;
	default:
		break;
	}

	
}

void Monster::UpdateIdle()
{
	//대기 상태일 때는 타겟을 찾는다.
	bool ret = SearchTarget();
	if (ret == true)
		SetState(CreatureState::Move);
}

void Monster::UpdateMove()
{
	//AI 이동은 언리얼 AI를 활용하자.
	//그래야 네비게이션 활용이 가능.

	//타겟이 중간에 죽거나, 사라졌으면, Idle 상태로 전환.
	if (_target == nullptr)
	{
		SetState(CreatureState::Idle);
		return;
	}else if (_target->GetState() == CreatureState::Dead)
	{
		UnBindTarget();
		SetState(CreatureState::Idle);
		return;
	}
	float dis = GetDistanceFromTarget(_target->GetPos());
	if (dis > GetInfo().stat().searchrange())
	{
		//만약 추적중에 상대방이 탐색거리를 벗어났다면, 
		UnBindTarget();
		SetState(CreatureState::Idle);
		return;
	}

	MoveTo(_target);
	//타깃과의 거리가 공격 거리 안으로 들어오면 공격한다.
	if (CanAttack() == true)
	{
		SetState(CreatureState::Attack);
	}

}

void Monster::UpdateAttack()
{	
	if (CanAttack() == true)
	{
		
		UseSkill(Protocol::Skill_ID::ATTACK);
	}else 
	{
		//만약 상대방이 공격 거리에서 벗어났다면, 
		SetState(CreatureState::Move);
	}
}

void Monster::UpdateDead()
{
}

void Monster::OnDead(Protocol::ObjectInfo damageCauser)
{
	Creature::OnDead(damageCauser);

	//죽으면 아이템을 떨구게 한다.
	FRewardData reward = GetRandomReward();
	Protocol::ItemInfo rewardItem = GDataManager->GetItemData(reward.Id);

	//떨군 아이템을 플레이어들한테 통지해준다.
	GetRoomRef()->EnterItem(rewardItem, GetPos());



	//Protocol::S_DropItem pkt;
	//pkt.mutable_iteminfo()->CopyFrom(rewardItem);
	//pkt.set_count(reward.Count);
	//SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt, Protocol::S_DROPITEM);
	//GetRoomRef()->BroadCast(sendBuffer);
}

FRewardData Monster::GetRandomReward()
{
	MonsterData data = GDataManager->GetMonsterData(_info.templateid());

	// 시드값을 얻기 위한 random_device 생성.
	std::random_device rd;
	// random_device 를 통해 난수 생성 엔진을 초기화 한다.
	std::mt19937 gen(rd());
	// 0 부터 99 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
	std::uniform_int_distribution<int> dis(0, 100);
	int rand = dis(gen);
	int sum = 0;
	for (FRewardData reward : data.RewardDatas)
	{
		sum += reward.ItemDropRate;
		if (rand <= sum)
		{
			return reward;
			break;
		}
	}
	return FRewardData();
}



bool Monster::CanAttack()
{
	if (_target->GetState() == CreatureState::Dead)
	{
		UnBindTarget();
		SetState(CreatureState::Idle);
		return false;
	}


	float dis = GetDistanceFromTarget(_target->GetPos());
	if (dis <= GetInfo().stat().attackrange())
	{
		return true;
	}
	return false;
}

void Monster::UseSkill(Protocol::Skill_ID skillId)
{
	_attackCoolTime += GetRoomRef()->_deltaTime;
	if (_attackCoolTime < 1)
		return;
	else
		_attackCoolTime = 0;

	Protocol::S_Skill pkt;
	Protocol::ObjectInfo* monster = pkt.mutable_info();
	monster->CopyFrom(GetInfo());
	pkt.set_skillid(skillId);
	SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt, Protocol::S_SKILL);
	GetRoomRef()->BroadCast(sendBuffer);


	//string text = std::format("{}가 {}를 {}공격력으로 공격합니다.", GetInfo().name(), _target->GetInfo().name(), _target->GetInfo().stat().damage());
	//cout << text << endl;
}

bool Monster::SearchTarget()
{
	GameRoomRef myRoom = GetRoomRef();
	if (myRoom == nullptr)
		return false;

	for (auto player : myRoom->_players)
	{
		float dis = GetDistanceFromTarget(player.second->GetPos());
		if (dis <= GetInfo().stat().searchrange())
		{
			BindTarget(player.second);
			return true;
		}
	}
	UnBindTarget();
	return false;
}

void Monster::MoveTo(shared_ptr<Creature> target)
{
	//타겟을 향해 움직이게 한다?
	Protocol::Position currentPos = GetPos();
	Protocol::Position targetPos = target->GetPos();
	float moveSpeed = GetInfo().stat().movespeed() * GetRoomRef()->_deltaTime;

	if(currentPos.locationx()> targetPos.locationx())
		currentPos.set_locationx(currentPos.locationx() - moveSpeed);
	if (currentPos.locationx() < targetPos.locationx())
		currentPos.set_locationx(currentPos.locationx() + moveSpeed);
	if (currentPos.locationy() > targetPos.locationy())
		currentPos.set_locationy(currentPos.locationy() - moveSpeed);
	if (currentPos.locationy() < targetPos.locationy())
		currentPos.set_locationy(currentPos.locationy() + moveSpeed);
	//if (currentPos.locationz() > targetPos.locationz())
	//	currentPos.set_locationz(currentPos.locationz() - moveSpeed);
	//if (currentPos.locationz() < targetPos.locationz())
	//	currentPos.set_locationz(currentPos.locationz() + moveSpeed);

	currentPos.set_velocityx(1);
	currentPos.set_velocityy(1);
	currentPos.set_velocityz(1);

	SetPos(currentPos);

	Protocol::S_Move pkt;
	Protocol::ObjectInfo* sInfo = pkt.mutable_info();
	sInfo->CopyFrom(GetInfo());
	SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pkt, S_MOVE);
	GetRoomRef()->BroadCast(sendBuffer);
}
