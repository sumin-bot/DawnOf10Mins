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
		// 1. 방향과 거리를 동시에 구합니다.
		FVector ToPlayer = PlayerPawn->GetActorLocation() - GetActorLocation();
		float Distance = ToPlayer.Size();
		FVector Direction = ToPlayer.GetSafeNormal();

		// 2. 거리 제한 (Dead-zone) 설정
		if (Distance > 15.f)
		{
			AddMovementInput(Direction, 1.0f);

			// 적이 플레이어를 바라보게 하고 싶다면 (회전 로직)
			FRotator TargetRot = Direction.Rotation();
			FRotator NewRot = FRotator(0.f, TargetRot.Yaw, 0.f);
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), NewRot, DeltaTime, 5.f));
		}
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