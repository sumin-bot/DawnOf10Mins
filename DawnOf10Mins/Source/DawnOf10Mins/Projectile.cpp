// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Kismet/GameplayStatics.h"
#include "Enemy.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 충돌체 설정
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = CollisionComp;
	CollisionComp->InitSphereRadius(5.f);

	// 발사체 이동 설정
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;

	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnHit);
}

void AProjectile::OnHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 1. 맞은 대상이 적(Enemy)인지 확인
	if (OtherActor && OtherActor != GetOwner() && OtherActor->IsA(AEnemy::StaticClass()))
	{
		// 2. 데미지 전달
		UGameplayStatics::ApplyDamage(OtherActor, 10.f, GetInstigatorController(), this, UDamageType::StaticClass());

		// 3. 맞췄으니 총알 삭제 (관통 기능을 넣고 싶다면 이 줄을 빼면 됩니다!)
		Destroy();
	}
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(2.f);
	
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

