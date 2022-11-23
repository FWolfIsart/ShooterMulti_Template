#include "PickupDirector.h"
#include "Engine/World.h"
#include "../GameFramework/DeathMatchGS.h"

APickupDirector* APickupDirector::Instance = nullptr;

APickupDirector::APickupDirector()
{
}

void APickupDirector::BeginPlay()
{
	Super::BeginPlay();

	if (!Instance)
		Instance = this;

	IsSpawnFullArray.SetNum(SpawnPoints.Num());
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APickupDirector::SpawnTick, SecondPerSpawn, true);

	ADeathMatchGS* GameState = Cast<ADeathMatchGS>(GetWorld()->GetGameState());
	GameState->OnPlayerNum.AddLambda([this](ADeathMatchGS* GS) { UpdateFrequencies(GS); });
}

void APickupDirector::SpawnTick()
{
	if (bIsFull)
		return;
	
	int MaxPoints = SpawnPoints.Num() - 1;
	int RandomPoint = FMath::RandRange(0, MaxPoints);
	int PrevPoint = RandomPoint;

	while (IsSpawnFullArray[RandomPoint])
	{
		RandomPoint = (RandomPoint + 1) % MaxPoints;
		if (RandomPoint == PrevPoint)
		{
			bIsFull = true;
			return;
		}
	}

	IsSpawnFullArray[RandomPoint] = true;
	SpawnPickup(CurrentPickupIndex, RandomPoint);
	CurrentPickupIndex = (CurrentPickupIndex + 1) % PickupBPs.Num();
}

void APickupDirector::SpawnPickup(int pickupIndex, int spawnPointIndex)
{
	auto pickupBP = PickupBPs[pickupIndex];
	auto pickupLocation = SpawnPoints[spawnPointIndex]->GetActorLocation();
	auto pickupRotation = SpawnPoints[spawnPointIndex]->GetActorRotation();

	auto Pickup = GetWorld()->SpawnActor<APickup>(pickupBP, pickupLocation, pickupRotation);

	if (Pickup)
	{
		Pickup->SpawnKey.ClassKey = pickupIndex;
		Pickup->SpawnKey.SpawnPointKey = spawnPointIndex;
	}
}

void APickupDirector::FreePickup(FSpawnKey Key)
{
	if (!Instance)
		return;

	Instance->IsSpawnFullArray[Key.SpawnPointKey] = false;
}

void APickupDirector::SetFull(bool isFull)
{
	bIsFull = isFull;
}

void APickupDirector::UpdateFrequencies(ADeathMatchGS* GameState)
{
	bIsFull = false;

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APickupDirector::SpawnTick, GameState->NewFrequency(SecondPerSpawn), true);
}
