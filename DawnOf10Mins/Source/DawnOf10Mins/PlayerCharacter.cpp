// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Enemy.h"
#include "Projectile.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APlayerCharacter::APlayerCharacter()
{
    // 카메라 설정 (Top Down 고정 뷰)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->TargetArmLength = 800.f;
    CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritYaw = false;
    CameraBoom->bInheritRoll = false;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom);

    // 캐릭터 회전 설정 (이동 방향을 바라보게)
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;

    // 센서 생성 및 설정
    DetectSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectSphere"));
    DetectSphere->SetupAttachment(RootComponent);
    DetectSphere->SetSphereRadius(AttackRange);
    DetectSphere->SetCollisionProfileName(TEXT("Trigger"));

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
    // 입력 컨텍스트 등록
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    FindTarget();

    // 타겟이 있다면 그쪽을 바라보게 함
    if (CurrentTarget)
    {
        GetCharacterMovement()->bOrientRotationToMovement = false;

        // 내 위치에서 타겟 위치를 향한 회전값 계산
        FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), CurrentTarget->GetActorLocation());

        // Z축(Yaw)만 회전하도록 설정
        FRotator NewRotation = FRotator(0.f, TargetRotation.Yaw, 0.f);

        // 부드럽게 회전 
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), NewRotation, DeltaTime, 10.f));

        if (GetWorld()->GetTimeSeconds() - LastFireTime > FireRate)
        {
            Fire();
            LastFireTime = GetWorld()->GetTimeSeconds();
        }
    }
    else
    {
        GetCharacterMovement()->bOrientRotationToMovement = true;
    }
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Move 함수를 IA_Move에 연결
    if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
    }

}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // 월드의 Forward(1,0,0)와 Right(0,1,0) 방향으로 이동
        AddMovementInput(FVector::ForwardVector, MovementVector.Y);
        AddMovementInput(FVector::RightVector, MovementVector.X);
    }
}

void APlayerCharacter::FindTarget()
{
    TArray<AActor*> OverlappingActors;
    DetectSphere->GetOverlappingActors(OverlappingActors, AEnemy::StaticClass());

    if (OverlappingActors.Num() == 0)
    {
        CurrentTarget = nullptr;
        return;
    }

    // 가장 가까운 적 찾기
    float NearestDistance = AttackRange;
    AActor* NearestActor = nullptr;

    for (AActor* Actor : OverlappingActors)
    {
        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestActor = Actor;
        }
    }
    CurrentTarget = NearestActor;
}

void APlayerCharacter::Fire()
{

    if (ProjectileClass)
    {
        FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f + GetActorUpVector() * 50.f;
        FRotator SpawnRotation = GetActorRotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
    }
}