// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientPacketHandler.h"
#include "Kismet/GameplayStatics.h"
#include "GameManager.h"
#include "Managers/ObjectManager.h"
#include "Managers/NetworkManager.h"
#include "Managers/UIManager.h"
#include "Managers/InventoryManager.h"
#include "MyPlayerController.h"
#include "BufferHelper.h"
#include "Buffer.h"
#include "Protocol.pb.h"
#include "MMORPG2GameModeBase.h"
#include "StatComponent.h"
#include "UI/TitleWidget.h"
#include "Components/TextBlock.h"
#include "Components/EditableText.h"
#include "Components/Button.h"
#include "MyAnimInstance.h"
#include "UI/ChatWidget.h"
#include "UI/InGameUI.h"
#include "UI/InventoryUI.h"
#include "UI/LobbyUI.h"
#include "UI/CharacterSelectListUI.h"
#include "UI/QuestUI.h"

void ClientPacketHandler::Init()
{

}

void ClientPacketHandler::OnRecvPacket(ServerSessionRef session, BYTE* buffer, int32 len)
{
   PacketHeader* header = (PacketHeader*)buffer;
//	uint16 id = header->id;
	uint16 size = header->size;

	PacketMessage message;
	message.pkt.AddUninitialized(len);
	FMemory::Memcpy(message.pkt.GetData(),buffer, len);
	GameInstance->GetPacketQueue()->Push(message);

	//FString string = FString::Printf(TEXT("Size[%d] Len[%d] TArray.num[%d]"), size, len, message.pkt.GetAllocatedSize());
	//Utils::DebugLog(string);
}

void ClientPacketHandler::HandlePacket(PacketMessage& packet)
{
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	uint16 id = header->id;

	switch (id)
	{
		case Protocol::S_TEST:
			Handle_S_TEST(packet);
			break;
		case Protocol::S_SPAWN_MY_PLAYER:
			Handle_S_SpawnMyPlayer(packet);
			break;
		case Protocol::S_ADDOBJECTS:
			Handle_S_AddObjects(packet);
			break;
		case Protocol::S_DEWSPAWN:
			Handle_S_Despawn(packet);
			break;
		case Protocol::S_DISCONNECT:
			Handle_S_Disconnect(packet);
			break;
		case Protocol::S_MOVE:
			Handle_S_Move(packet);
			break;
		case Protocol::S_SKILL:
			Handle_S_Skill(packet);
			break;
		case Protocol::S_CHANGEDHP:
			Handle_S_ChangedHP(packet);
			break;
		case Protocol::S_CONNECT:
			Handle_S_Connect(packet);
			break;
		case Protocol::S_SUCCESSLOGIN:
			Handle_S_SuccessLogin(packet);
			break;
		case Protocol::S_CREATEACCOUNT:
			Handle_S_CreateAccount(packet);
			break;
		case Protocol::S_FAILEDLOGIN:
			Handle_S_FailedLogin(packet);
			break;
		case Protocol::S_CHAT:
			Handle_S_Chat(packet);
			break;
		case Protocol::S_DIE:
			Handle_S_Die(packet);
			break;
		case Protocol::S_PICKUPITEM:
			Handle_S_PickUpItem(packet);
			break;
		case Protocol::S_UPDATE_INFO:
			Handle_S_UpdateInfo(packet);
			break;
		case Protocol::S_ADD_TO_INVENTORY:
			Handle_S_AddToInventory(packet);
			break;
		case Protocol::S_EQUIPPED_ITEM:
			Handle_S_EquippedItem(packet);
			break;
		case Protocol::S_LOADING_CHARACTERLIST:
			Handle_S_LoadingCharacterList(packet);
			break;
		case Protocol::S_REFRESH_INVENTORY:
			Handle_S_RefreshInventory(packet);
			break;
		case Protocol::S_USE_ITEM:
			Handle_S_UseItem(packet);
			break;
		case Protocol::S_BUY_ITEM:
			Handle_S_BuyItem(packet);
			break;
		case Protocol::S_GET_QUEST:
			Handle_S_GetQuest(packet);
			break;
		case Protocol::S_QUEST_UPDATE:
			Hande_S_UpdateQuest(packet);
			break;
	default:
		break;
	}
}



void ClientPacketHandler::Handle_S_TEST(PacketMessage& packet)
{
	Protocol::S_Test pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));
	int32 id = pkt.id();
	int32 hp = pkt.hp();
	int32 attack = pkt.attack();

	FString text = FString::Printf(TEXT("%d %d %d"), id, hp, attack);
	Utils::DebugLog(text);
}

void ClientPacketHandler::Handle_S_SpawnMyPlayer(PacketMessage& packet)
{
	Protocol::S_SpawnMyPlayer pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	int32 id = pkt.info().id();
	FString name = *FString(pkt.info().name().c_str());

	FString debugString = FString::Printf(TEXT("ID[%d] Name [%s]\n"), id, *name);
	Utils::DebugLog(debugString);


	GameInstance->GetObjectManager()->CreateMyPlayer(pkt.info());
}

void ClientPacketHandler::Handle_S_AddObjects(PacketMessage& packet)
{
	//오브젝트 생성.
	Protocol::S_AddObjects pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	for (Protocol::ObjectInfo info : pkt.objects())
	{

		GameInstance->GetObjectManager()->CreateObject(info);

	}

}

void ClientPacketHandler::Handle_S_Despawn(PacketMessage& packet)
{
	Protocol::S_Despawn pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	for (Protocol::ObjectInfo info : pkt.objects())
	{
		GameInstance->GetObjectManager()->DespawnObject(&info);
	}

}

void ClientPacketHandler::Handle_S_Die(PacketMessage& packet)
{
	Protocol::S_Die pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	ACreature* creature = GameInstance->GetObjectManager()->GetPlayerByID(pkt.target().id());
	creature->OnDead();
}

void ClientPacketHandler::Handle_S_Disconnect(PacketMessage& packet)
{

	Protocol::S_Disconnect pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));
	FString text = FString::Printf(TEXT("ID[% d], Name[% s]의 연결이 끊어졌습니다.\n"), pkt.info().id(), pkt.info().name().c_str());
	Utils::DebugLog(text);


}

void ClientPacketHandler::Handle_S_Move(PacketMessage& packet)
{
	Protocol::S_Move pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));
	ACreature* player=  GameInstance->GetObjectManager()->GetPlayerByID(pkt.info().id());
	if (IsValid(player) == false)
	{
		Utils::DebugLog(FString::Printf(TEXT("Handle S Move 없는 ID[%d] 접근 시도.\n "), pkt.info().id()));
		return;
	}
	if (player == GameInstance->GetObjectManager()->GetMyPlayer())
		return;


	player->UpdateInfo(pkt.mutable_info());

}

void ClientPacketHandler::Handle_S_Skill(PacketMessage& packet)
{
	Protocol::S_Skill pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	//스킬 패킷 안에는 플레이어와 사용한 스킬 ID 값이 들어 있다.
	ACreature* player = GameInstance->GetObjectManager()->GetPlayerByID(pkt.info().id());
	//FString log = FString::Printf(TEXT("%s가 스킬을 사용합니다."), *player->GetName());
	//Utils::DebugLog(log);
	if (IsValid(player) == false)
		return;

	player->UpdatePlayerSkill(pkt.skillid(), pkt.attackindex());
}

void ClientPacketHandler::Handle_S_ChangedHP(PacketMessage& packet)
{
	Protocol::S_ChangedHP pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));


	//
	ACreature* player = GameInstance->GetObjectManager()->GetPlayerByID(pkt.target().id());
	if(IsValid(player) == true)
		player->Stat->SetHp(pkt.target().stat().hp());
	if (IsValid(player) == false)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Handle S_Changed HP Error"));
	}
}

void ClientPacketHandler::Handle_S_Chat(PacketMessage& packet)
{
	Protocol::S_Chat pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));


	//채팅이 날아왔다.
	FString text = UTF8_TO_TCHAR(pkt.msg().c_str());
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, *text);

	UInGameUI* UI = Cast<UInGameUI>(GameInstance->GetUIManager()->GetMainUI());
	if (IsValid(UI) == false)
	{
		Utils::DebugLog(TEXT("Handle_S_Chat Error. UInGameUI를 찾을 수 없습니다."));
		return;
	}

	UI->ChatUI->UpdateChat(text);


}

void ClientPacketHandler::Handle_S_PickUpItem(PacketMessage& packet)
{
	Protocol::S_PickUpItem pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	//누가 먹었는지 플레이어를 찾는다.
	ARPGPlayer* player = Cast<ARPGPlayer>(GameInstance->GetObjectManager()->GetPlayerByID(pkt.info().id()));
	if (IsValid(player) == false)
	{
		Utils::DebugLog("Handle S PickUpItem Error");
		return;
	}
	AMyItem* pickedItem = GameInstance->GetObjectManager()->GetItemByID(pkt.pickitem().id());
	if (IsValid(pickedItem) == false)
	{
		Utils::DebugLog("Handle S PickUpItem Error");
		return;
	}
	//먹은 사람이 자신일 경우 인벤토리에 넣어준다.
	if(pkt.info().id() == GameInstance->GetObjectManager()->GetMyPlayer()->GetInfo()->id())
		player->AddToInventory(pkt.mutable_iteminfo());
	//아이템을 먹었다면 필드에서 지워준다.
	GameInstance->GetObjectManager()->RemoveItemByID(pickedItem->GetObjectInfo().id());
}

void ClientPacketHandler::Handle_S_UpdateInfo(PacketMessage& packet)
{
	Protocol::S_UpdateInfo pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	ACreature* creature = GameInstance->GetObjectManager()->GetPlayerByID(pkt.info().id());
	if (IsValid(creature) == false)
		return;
	creature->UpdateInfo(pkt.mutable_info());

}

void ClientPacketHandler::Handle_S_AddToInventory(PacketMessage& packet)
{
	Protocol::S_AddToInventory pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	for (auto item : pkt.items())
	{
		GameInstance->GetInventory()->AddItem(&item);
	}

	UInGameUI* ui = Cast<UInGameUI>(GameInstance->GetUIManager()->GetMainUI());
	if (IsValid(ui) == false)
	{
		Utils::DebugLog("Handle_S_AddToInventory Error");
		return;
	}

	ui->InventoryUI->RefreshUI();

}

void ClientPacketHandler::Handle_S_RefreshInventory(PacketMessage& packet)
{
	Protocol::S_RefreshInventory pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));


	GameInstance->GetInventory()->Items.Empty();
	for (auto item : pkt.items())
	{
		GameInstance->GetInventory()->AddItem(&item);
	}

	UInGameUI* ui = Cast<UInGameUI>(GameInstance->GetUIManager()->GetMainUI());
	if (IsValid(ui) == false)
	{
		Utils::DebugLog("Handle_S_AddToInventory Error");
		return;
	}

	ui->InventoryUI->Gold = pkt.gold();
	ui->InventoryUI->RefreshUI();
}

void ClientPacketHandler::Handle_S_EquippedItem(PacketMessage& packet)
{
	Protocol::S_EquippedItem pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	//누가 먹었는지 플레이어를 찾는다.
	ARPGPlayer* player = Cast<ARPGPlayer>(GameInstance->GetObjectManager()->GetPlayerByID(pkt.playerinfo().id()));
	if (IsValid(player) == false)
	{
		Utils::DebugLog("Handle S PickUpItem Error");
		return;
	}

	player->EquippedItem(pkt.mutable_iteminfo());

}

void ClientPacketHandler::Handle_S_UseItem(PacketMessage& packet)
{
	Protocol::S_UseItem pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));


	//누가 먹었는지 플레이어를 찾는다.
	ARPGPlayer* player = Cast<ARPGPlayer>(GameInstance->GetObjectManager()->GetPlayerByID(pkt.playerinfo().id()));
	if (IsValid(player) == false)
	{
		Utils::DebugLog("Handle S Use Item");
		return;
	}
}

void ClientPacketHandler::Handle_S_BuyItem(PacketMessage& packet)
{
	Protocol::S_BuyItem pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));


}

void ClientPacketHandler::Handle_S_GetQuest(PacketMessage& packet)
{
	Protocol::S_GetQuest pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	//업데이트 해주기.
	UInGameUI* ui = Cast<UInGameUI>(GameInstance->GetUIManager()->GetMainUI());
	if (IsValid(ui) == false)
	{
		Utils::DebugLog("Handle_S_GetQuest Error");
		return;
	}

	ui->QuestUI->AddQuest(pkt.questtemplatedid(), pkt.currentprogress());
	//ui->QuestUI->RefreshUI();
}

void ClientPacketHandler::Hande_S_UpdateQuest(PacketMessage& packet)
{
	Protocol::S_UpdateQuest pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	//업데이트 해주기.
	UInGameUI* ui = Cast<UInGameUI>(GameInstance->GetUIManager()->GetMainUI());
	if (IsValid(ui) == false)
	{
		Utils::DebugLog("Hande_S_UpdateQuest Error");
		return;
	}

	FQuestData* data = ui->QuestUI->GetQuestData(pkt.questtemplatedid());
	if (data == nullptr)
		return;

	data->CurrentProgress = pkt.currentprogress();
	ui->QuestUI->RefreshUI();
}

void ClientPacketHandler::Handle_S_Connect(PacketMessage& packet)
{
	Protocol::S_Connect pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	//서버에서 보내준 String  파싱
	//FString text = FString::Printf(TEXT("%s"), pkt.info().c_str());
	FString text = UTF8_TO_TCHAR(pkt.info().c_str());
	//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, *text);

	//연결이 되었다. 이 시점에서 현재 사용자의 위치는 Title 화면.
	//Title 화면에 어떻게 정보를 전달해야할까?
	//일단 BP_Title을 가져와야 한다.
	UWorld* world = GameInstance->GetWorld();
	AMMORPG2GameModeBase* mode = Cast<AMMORPG2GameModeBase>(UGameplayStatics::GetGameMode(world));
	if (IsValid(mode) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_S_Connect ERROR! Don't find mode!"));
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Handle_S_Connect ERROR! Don't find mode!"));
		return;
	}

	UTitleWidget* title = Cast<UTitleWidget>(mode->MainUI);
	if (IsValid(title) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_S_Connect ERROR! Don't find Title Widget"));
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Handle_S_Connect ERROR! Don't find Title Widget"));
		return;
	}

	FText serverText = FText::FromString(text);
	title->Information_Text->SetText(serverText);
	title->CurrentState = TitleState::Login;
	title->IP_EditableText->SetVisibility(ESlateVisibility::Hidden);
	title->ID_EditableText->SetVisibility(ESlateVisibility::Visible);
	title->Password_EditableText->SetVisibility(ESlateVisibility::Visible);
	title->CreateAccount_Button->SetVisibility(ESlateVisibility::Visible);
	//타이틀 화면에 서버 연결이 되었다는 정보를 전달한 후, 

	//로그인 절차에 들어간다.

	//이후 로그인 절차가 끝나면 레벨 이동 후 캐릭터 생성
}

void ClientPacketHandler::Handle_S_SuccessLogin(PacketMessage& packet)
{
	Protocol::S_SuccessLogin pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));


	//서버에서 날아온 string 변환
	FString text = UTF8_TO_TCHAR(pkt.info().c_str());

	//현재 화면의 UI 가져오기
	UWorld* world = GameInstance->GetWorld();
	AMMORPG2GameModeBase* mode = Cast<AMMORPG2GameModeBase>(UGameplayStatics::GetGameMode(world));
	if (IsValid(mode) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_S_Connect ERROR! Don't find mode!"));
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Handle_S_Connect ERROR! Don't find mode!"));
		return;
	}

	UTitleWidget* title = Cast<UTitleWidget>(mode->MainUI);
	if (IsValid(title) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Handle_S_Connect ERROR! Don't find Title Widget"));
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Handle_S_Connect ERROR! Don't find Title Widget"));
		return;
	}

	FText serverText = FText::FromString(text);
	title->Information_Text->SetText(serverText);

	//이 단계에 들어갔다는 건 서버에서 아이디와 비밀번호를 받고, 로그인에 성공했다는 것.
	//Title 상태를 바꿔서, Login 확인 패킷을 여러번 보낼 수 없게 만들어준다.
	title->CurrentState = TitleState::LoginSuccess;
	GameInstance->LoadGameLevel(FString(TEXT("Lobby")));
}

void ClientPacketHandler::Handle_S_CreateAccount(PacketMessage& packet)
{
	Protocol::S_CreateAccount pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	//서버에서 날아온 string 변환
	FString text = UTF8_TO_TCHAR(pkt.msg().c_str());
	//현재 화면의 UI 가져오기
	UWorld* world = GameInstance->GetWorld();
	AMMORPG2GameModeBase* mode = Cast<AMMORPG2GameModeBase>(UGameplayStatics::GetGameMode(world));
	UTitleWidget* title = Cast<UTitleWidget>(mode->MainUI);
	FText serverText = FText::FromString(text);
	title->Information_Text->SetText(serverText);

	//계정 생성 실패했다면 다시 만들기.
	if (pkt.result() == false)
		return;

	//성공적으로 끝났으면 로그인 후 게임 접속
	title->GoToConnectState();
	//FTimerHandle WaitHandle;
	//float WaitTime = 1.0f; //시간을 설정하고
	//GameInstance->GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
	//	{

	//		GameInstance->LoadGameLevel(FString(TEXT("Game")));
	//		Make_C_EnterRoom();
	//	}), WaitTime, false);
}

void ClientPacketHandler::Handle_S_FailedLogin(PacketMessage& packet)
{
	Protocol::S_FailedLogin pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	//서버에서 날아온 string 변환
	FString text = UTF8_TO_TCHAR(pkt.info().c_str());
	//현재 화면의 UI 가져오기
	UWorld* world = GameInstance->GetWorld();
	AMMORPG2GameModeBase* mode = Cast<AMMORPG2GameModeBase>(UGameplayStatics::GetGameMode(world));
	UTitleWidget* title = Cast<UTitleWidget>(mode->MainUI);
	FText serverText = FText::FromString(text);
	title->Information_Text->SetText(serverText);
}

void ClientPacketHandler::Handle_S_LoadingCharacterList(PacketMessage& packet)
{
	Protocol::S_LoadingCharacterList pkt;
	PacketHeader* header = (PacketHeader*)packet.pkt.GetData();
	pkt.ParseFromArray(&header[1], header->size - sizeof(PacketHeader));

	ULobbyUI* UI = Cast<ULobbyUI>(GameInstance->GetUIManager()->GetMainUI());
	if (IsValid(UI) == false)
	{
		Utils::DebugLog(TEXT("Handle_S_LoadingCharacterList Error"));
		return;
	}

	Utils::DebugLog(TEXT("Handle_S_LoadingCharacterList"));
	UI->CharacterList->CharacterInfos.Empty();
	for (auto element : pkt.characterlist())
	{
		Protocol::CharacterListElement ele;
		ele.CopyFrom(element);
		UI->CharacterList->CharacterInfos.Add(ele);
	}
	UI->CharacterList->RefreshUI();
}


SendBufferRef ClientPacketHandler::Make_S_TEST(uint64 id, uint32 hp, uint16 attack)
{
	Protocol::S_Test pkt;

	pkt.set_id(10);
	pkt.set_hp(100);
	pkt.set_attack(10);


	return MakeSendBuffer(pkt, Protocol::S_TEST);
}

void ClientPacketHandler::Make_C_Move(Protocol::ObjectInfo info)
{
	Protocol::C_Move pkt;
	Protocol::ObjectInfo* pktInfo = pkt.mutable_info();
	*pktInfo = info;
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_MOVE);

	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);
}

void ClientPacketHandler::Make_C_ChangedHP(Protocol::ObjectInfo target, Protocol::ObjectInfo damageCauser, int32 damageAmount)
{
	Protocol::C_ChangedHP pkt;
	Protocol::ObjectInfo* pkt_Target = pkt.mutable_target();
	Protocol::ObjectInfo* pkt_DamageCauser = pkt.mutable_damagecauser();

	pkt_Target->CopyFrom(target);
	pkt_DamageCauser->CopyFrom(damageCauser);
	pkt.set_damageamount(damageAmount);
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_CHANGEDHP);
	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);
}

void ClientPacketHandler::Make_C_Chat(FString msg)
{
	Protocol::C_Chat pkt;
	int32 playerId = GameInstance->GetObjectManager()->GetMyPlayer()->GetInfo()->id();
	pkt.set_id(playerId);
	pkt.set_msg(TCHAR_TO_UTF8(*msg));

	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_CHAT);
	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);
}

void ClientPacketHandler::Make_C_PickUpItem(Protocol::ObjectInfo info, Protocol::ObjectInfo pickItem)
{
	Protocol::C_PickUpItem pkt;
	pkt.mutable_info()->CopyFrom(info);
	pkt.mutable_pickitem()->CopyFrom(pickItem);


	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_PICKUPITEM);
	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);

}

void ClientPacketHandler::Make_C_MonsterMove(Protocol::ObjectInfo info)
{
	Protocol::C_MonsterMove pkt;
	Protocol::ObjectInfo* pktInfo = pkt.mutable_info();
	*pktInfo = info;
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_MONSTER_MOVE);

	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);
}

void ClientPacketHandler::Make_C_AddToInventory(Protocol::ObjectInfo player, Protocol::ObjectInfo itemInfo)
{
	Protocol::C_AddToInventory pkt;
	pkt.mutable_playerinfo()->CopyFrom(player);
	pkt.mutable_iteminfo()->CopyFrom(itemInfo);
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_ADD_TO_INVENTORY);

	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);
}

void ClientPacketHandler::Make_C_EquippedItem(Protocol::ObjectInfo player, Protocol::ItemInfo itemInfo)
{
	Protocol::C_EquippedItem pkt;
	pkt.mutable_playerinfo()->CopyFrom(player);
	pkt.mutable_iteminfo()->CopyFrom(itemInfo);
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_EQUIPPED_ITEM);

	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);

}

void ClientPacketHandler::Make_C_UseItem(Protocol::ObjectInfo player, Protocol::ItemInfo itemInfo)
{
	Protocol::C_UseItem pkt;
	pkt.mutable_playerinfo()->CopyFrom(player);
	pkt.mutable_iteminfo()->CopyFrom(itemInfo);
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_USE_ITEM);

	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);


}

void ClientPacketHandler::Make_C_BuyItem(int32 itemTemplatedId)
{
	Protocol::C_BuyItem pkt;
	pkt.set_itemtemplatedid(itemTemplatedId);
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_BUY_ITEM);
	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);
}

void ClientPacketHandler::Make_C_SpawnBoss()
{
	Protocol::C_Spanw_Boss pkt;
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_SPAWN_BOSS);
	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);

}

void ClientPacketHandler::Make_C_GetQuest(int32 questTemplatedId)
{
	Protocol::C_GetQuest pkt;
	pkt.set_questtemplatedid(questTemplatedId);
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_GET_QUEST);
	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);
}


void ClientPacketHandler::Make_C_TryLogin(FString id, FString password)
{
	Protocol::C_TryLogin pkt;
	pkt.set_id(TCHAR_TO_UTF8(*id));
	pkt.set_password(TCHAR_TO_UTF8(*password));
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_TRYLOGIN);
	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);
}

void ClientPacketHandler::Make_C_EnterRoom(int32 characterId)
{
	Protocol::C_EnterRoom pkt;
	pkt.set_characterid(characterId);
	SendBufferRef sendbuffer = MakeSendBuffer(pkt, Protocol::C_ENTER_ROOM);

	GameInstance->GetNetworkManager()->SendPacket(sendbuffer);

}

void ClientPacketHandler::Make_C_CreateAccount(FString id, FString password)
{
	Protocol::C_CreateAccount pkt;
	pkt.set_id(TCHAR_TO_UTF8(*id));
	pkt.set_password(TCHAR_TO_UTF8(*password));
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_CREATEACCOUNT);
	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);
}

void ClientPacketHandler::Make_C_LoadingCharacterList()
{
	Protocol::C_LoadingCharacterList pkt;
	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_LOADING_CHARACTERLIST);
	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);
}

void ClientPacketHandler::Make_C_CreateCharacter(int32 templatedId, FString name)
{
	Protocol::C_CreateCharacter pkt;
	pkt.set_templatedid(templatedId);

	string characterName;
	WideCharToMultiByte(CP_ACP, 0, *name, -1, characterName.data(), 256, NULL, NULL);
	pkt.set_name(characterName.c_str());

	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_CREATE_CHARACTER);
	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);

}

void ClientPacketHandler::Make_C_DeleteCharacter(int32 slot)
{
	Protocol::C_DeleteCharacter pkt;
	pkt.set_slot(slot);

	SendBufferRef sendBuffer = MakeSendBuffer(pkt, Protocol::C_DELETE_CHARACTER);
	GameInstance->GetNetworkManager()->SendPacket(sendBuffer);

}
