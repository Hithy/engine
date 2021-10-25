#include "system_physics.h"

#include "world.h"
#include "component_trans.h"
#include "component_physics.h"

namespace ECS {
 physx::PxMaterial* gMaterial = NULL;

void SystemPhysics::Tick(float dt) {
  auto ents = _scene->GetEntitiesByTypeExt(ComponentType_Physics);
  for (auto ent : ents) {
    auto comp_physics = dynamic_cast<ComponentPhysics*>(ent->GetComponent(ComponentType_Physics));
    if (!comp_physics->IsLoaded()) {
      LoadBody(ent);
    }
  }

  _accumulator += dt;
  if (_accumulator < _frame_rate)
  {
    return;
  }
  _accumulator -= _frame_rate;

  _scene->GetPxScene()->simulate(_frame_rate);
  _scene->GetPxScene()->fetchResults(true);

  for (auto ent : ents) {
    auto comp_physics = dynamic_cast<ComponentPhysics*>(ent->GetComponent(ComponentType_Physics));
    auto comp_trans = dynamic_cast<ComponentTransform*>(ent->GetComponent(ComponentType_Transform));

    auto px_pos = comp_physics->GetPosition();
    auto px_quat = comp_physics->GetRotation();
    comp_trans->SetPosition(px_pos);
    comp_trans->SetRotation(px_quat);
  }
}
void SystemPhysics::Start()
{
  if (!gMaterial) {
    gMaterial = World::GetInstance().GetPhysics()->createMaterial(0.5f, 0.5f, 0.6f);
  }
}
void SystemPhysics::Stop()
{
}

static physx::PxShape* CreateShape(physx::PxPhysics* physics, const glm::vec3& scale, int type) {
  if (type == physx::PxGeometryType::eSPHERE) {
    return physics->createShape(physx::PxSphereGeometry(scale.x), *gMaterial);
  } else if (type == physx::PxGeometryType::eBOX) {
    return physics->createShape(physx::PxBoxGeometry(scale.x, scale.y, scale.z), *gMaterial);
  }
  return nullptr;
}

void SystemPhysics::LoadBody(Entity* ent)
{
  auto comp_trans = dynamic_cast<ComponentTransform*>(ent->GetComponent(ComponentType_Transform));
  auto comp_physics = dynamic_cast<ComponentPhysics*>(ent->GetComponent(ComponentType_Physics));

  auto trans = comp_trans->GetPosition();
  auto scale = comp_physics->GetGeoSize();

  auto physics = World::GetInstance().GetPhysics();
  physx::PxRigidActor* new_body;
  if (comp_physics->IsStatic()) {
    new_body = physics->createRigidStatic(physx::PxTransform(trans.x, trans.y, trans.z));
  } else {
    new_body = physics->createRigidDynamic(physx::PxTransform(trans.x, trans.y, trans.z));
  }
  auto shape = CreateShape(physics, scale, comp_physics->GetGeoType());
  new_body->attachShape(*shape);
  shape->release();
  _scene->GetPxScene()->addActor(*new_body);
  comp_physics->SetActor(new_body);
  comp_physics->SetKinematic(comp_physics->IsKinematic());
}
}
