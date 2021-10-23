import _engine
# import numpy
from ecs import rotate_system

def createDirectionLight(color, direction, enable_shadow=False):
	comp_light = _engine.ComponentLight(1)
	comp_light.SetLightColor(color)
	comp_light.SetEnableShadow(enable_shadow)

	comp_trans = _engine.CreateComponentTransform()
	comp_trans.SetForward(direction)

	ent = _engine.CreateEntity()
	ent.AddComponent(comp_light)
	ent.AddComponent(comp_trans)

	return ent

def createPointLight(color, pos, enable_shadow=False):
	comp_light = _engine.ComponentLight(2)
	comp_light.SetLightColor(color)
	comp_light.SetEnableShadow(enable_shadow)

	comp_trans = _engine.CreateComponentTransform()
	comp_trans.SetPosition(pos)

	ent = _engine.CreateEntity()
	ent.AddComponent(comp_light)
	ent.AddComponent(comp_trans)

	return ent

def createBall(material, pos, size):
	comp_model = _engine.CreateComponentModel("resource/models/ball/ball.obj")
	comp_model.SetAlbedoPath("resource/images/pbr/" + material + "/albedo.png")
	comp_model.SetNormalPath("resource/images/pbr/" + material + "/normal.png")
	comp_model.SetMetalicPath("resource/images/pbr/" + material + "/metallic.png")
	comp_model.SetRouphnessPath("resource/images/pbr/" + material + "/roughness.png")
	comp_model.SetAOPath("resource/images/pbr/" + material + "/ao.png")

	comp_trans = _engine.CreateComponentTransform()
	comp_trans.SetPosition(pos)
	comp_trans.SetScale([size, size, size])
	# comp_trans.SetRotationEular([1.600, 4.660, 0.1])

	ent = _engine.CreateEntity()
	ent.AddComponent(comp_model)
	ent.AddComponent(comp_trans)

	return ent

def createBackpack(pos):
	comp_model = _engine.CreateComponentModel("resource/models/backpack/backpack.obj")
	comp_trans = _engine.CreateComponentTransform()
	comp_trans.SetPosition(pos)
	comp_trans.SetScale([0.01, 0.01, 0.01])
	# comp_trans.SetRotationEular([1.600, 4.660, 0.1])

	ent = _engine.CreateEntity()
	ent.AddComponent(comp_model)
	ent.AddComponent(comp_trans)

	return ent

def CreateCamera():
	comp_cam = _engine.ComponentCamera()
	comp_trans = _engine.CreateComponentTransform()

	cam_ent = _engine.CreateEntity()
	cam_ent.AddComponent(comp_cam)
	cam_ent.AddComponent(comp_trans)

	return cam_ent

class PyScene(_engine.Scene):
	def __init__(self):
		super().__init__()
		self._sys_list = []

	def tick(self, dt):
		for sys in self._sys_list:
			sys.tick(dt)

	def Init(self):
		# enable IBL skybox
		self.SetIBLPath("resource/images/Chelsea_Stairs/Chelsea_Stairs_3k.hdr")

		# systems
		self.add_system(_engine.CreateSystemCamera())
		self.add_system(_engine.CreateSystemModel())
		self.add_system(_engine.CreateSystemInput())
		self.add_system(_engine.CreateSystemSyncRender())
		self.add_system(rotate_system.RotateSystem())

		# camera
		cam_ent = CreateCamera()
		self.AddEntity(cam_ent)
		self.SetActiveCamera(cam_ent.get_id())

		# objects
		self.AddEntity(createBall("rusted_iron", [-7.0, 0.0, -15.0], 0.5))
		self.AddEntity(createBall("gold", [-4.0, 0.0, -15.0], 0.5))
		self.AddEntity(createBall("grass", [-1.0, 0.0, -15.0], 0.5))
		self.AddEntity(createBall("plastic", [2.0, 0.0, -15.0], 0.5))
		self.AddEntity(createBall("wall", [5.0, 0.0, -15.0], 0.5))
		self.AddEntity(createBackpack([5.0, 1.0, -18.0]))

		# light
		self.AddEntity(createDirectionLight([10.0, 10.0, 10.0], [1.0, 0.0, -0.2]))
		self.AddEntity(createPointLight([2000.0, 2000.0, 2000.0], [-5.0, 3.0, -15.0], 1))

	def add_system(self, sys):
		super().AddSystem(sys)
		self._sys_list.append(sys)
		sys.start()

