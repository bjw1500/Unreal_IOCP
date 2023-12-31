// Fill out your copyright notice in the Description page of Project Settings.



#include "MyItem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Network/Protocol.pb.h"
#include "Network/GameManager.h"
#include "Managers/DataTableManager.h"
#include "GameStruct.h"
#include "Creature.h"
#include "RPGPlayer.h"

// Sets default values
AMyItem::AMyItem()
{
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	ItemMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh Com"));

	SphereCollision->SetupAttachment(RootComponent);
	ItemMeshComponent->SetupAttachment(SphereCollision);

	SphereCollision->SetCollisionProfileName(TEXT("Item"));
	SphereCollision->SetSphereRadius(10.0f);
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AMyItem::OnCharacterOverlap);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &AMyItem::OnCharacterOverlapEnd);

}

// Called when the game starts or when spawned
void AMyItem::BeginPlay()
{
	Super::BeginPlay();
	//TODO
	//나중에 데이터 시트를 만들어서 서버에서 뿌려주는 정보대로 로드되게 해보자.
	//LoadItemInfo(1);
}

void AMyItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();

}

void AMyItem::OnCharacterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//플레이어가 아이템을 지니고 있는 상태라면,
	if (IsValid(Master) == true)
		return;
	ARPGPlayer* player = Cast<ARPGPlayer>(OtherActor);

	if (IsValid(player) == false)
		return;
	if (player->ThisMasterOtherClient == true)
		return;

	player->FindNearItem(this);
}

void AMyItem::OnCharacterOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	//플레이어가 아이템을 지니고 있는 상태라면,
	if (IsValid(Master) == true)
		return;
	ARPGPlayer* player = Cast<ARPGPlayer>(OtherActor);

	if (IsValid(player) == false)
		return;
	if (player->ThisMasterOtherClient == true)
		return;

	player->LoseNearItem(this);
}

void AMyItem::LoadItemInfo(int32 id)
{

	if(ItemMesh == nullptr)
		ItemMesh = Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), NULL, TEXT("/Game/InfinityBladeWeapons/Weapons/Blade/Swords/Blade_BlackKnight/SK_Blade_BlackKnight.SK_Blade_BlackKnight")));


	if (IsValid(ItemMesh) == false)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Load Item Info Failed"));
	}

	if (ItemMesh && ItemMeshComponent)
	{
		ItemMeshComponent->SetSkeletalMesh(ItemMesh);
	}

	//Item Info
	Protocol::ItemInfo info;
	info.set_name("Blade");
	SetItemInfo(info);
}

void AMyItem::SetItemData(FItemData* info)
{
	ItemData = *info;


	//ItemInfo 설정
	{
		_itemInfo.set_templateid(info->Id);
		_itemInfo.set_damage(info->Damage);
		_itemInfo.set_name(std::string(TCHAR_TO_UTF8 (*info->Name)));
		_itemInfo.set_armor(info->Armor);

		switch (info->ItemType)
		{
		case WEAPON:
			_itemInfo.set_itemtype(Protocol::WEAPON);
			break;
		case ARMOR:
			_itemInfo.set_itemtype(Protocol::ARMOR);
			break;
		case COUNSUMABLE:
			_itemInfo.set_itemtype(Protocol::CONSUMEABLE);
			break;
		default:
			break;
		}
	}
}

