// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 적의 캡슐 컴포넌트에 충돌 이벤트 연결
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnPlayerOverlapBegin);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnPlayerOverlapEnd);

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// 적의 회전 설정: 이동 방향을 바라보게
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 480.f, 0.f);

	Health = MaxHealth;

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	// 이동 속도 적용
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (PlayerPawn)
	{
		// 2. 방향 계산 (플레이어 위치 - 내 위치)
		FVector Direction = PlayerPawn->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.f; // 높이 차이는 무시
		Direction.Normalize();

		// 3. 이동 입력
		AddMovementInput(Direction, 1.f);
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health -= DamageApplied;
	UE_LOG(LogTemp, Warning, TEXT("Enemy HP: %f"), Health);

	if (Health <= 0.f)
	{
		Destroy(); // 사망
	}

	return DamageApplied;
}

void AEnemy::OnPlayerOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player && OtherComp == Player->GetRootComponent())
	{
		OverlappedPlayer = Player;
		// 1. 즉시 한 번 데미지를 줍니다.
		ApplyContinuousDamage();

		// 2. 이후 DamageInterval 간격으로 반복 실행되는 타이머 시작
		GetWorldTimerManager().SetTimer(DamageTimerHandle, this, &AEnemy::ApplyContinuousDamage, DamageInterval, true);
	}
}

void AEnemy::OnPlayerOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == OverlappedPlayer)
	{
		OverlappedPlayer = nullptr;
		// 3. 플레이어가 범위를 벗어나면 타이머 종료
		GetWorldTimerManager().ClearTimer(DamageTimerHandle);
	}
}

void AEnemy::ApplyContinuousDamage()
{
	if (OverlappedPlayer)
	{
		UGameplayStatics::ApplyDamage(OverlappedPlayer, Damage, GetController(), this, UDamageType::StaticClass());
	}
}