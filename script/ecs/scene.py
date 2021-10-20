import _engine
# import numpy
from ecs import rotate_system

def createPointLight(color, pos):
	comp_light = _engine.ComponentLight(2)
	comp_light.SetLightColor(color)

	comp_trans = _engine.CreateComponentTransform()
	comp_trans.SetPosition(pos)

	ent = _engine.CreateEntity()
	ent.AddComponent(comp_light)
	ent.AddComponent(comp_trans)

	return ent

def createBall(material, pos):
	comp_model = _engine.CreateComponentModel("resource/models/ball/ball.obj")
	comp_model.SetAlbedoPath("resource/images/pbr/" + material + "/albedo.png")
	comp_model.SetNormalPath("resource/images/pbr/" + material + "/normal.png")
	comp_model.SetMetalicPath("resource/images/pbr/" + material + "/metallic.png")
	comp_model.SetRouphnessPath("resource/images/pbr/" + material + "/roughness.png")
	comp_model.SetAOPath("resource/images/pbr/" + material + "/ao.png")

	comp_trans = _engine.CreateComponentTransform()
	comp_trans.SetPosition(pos)
	comp_trans.SetScale([0.5, 0.5, 0.5])
	# comp_trans.SetRotationEular([1.600, 4.660, 0.1])

	ent = _engine.CreateEntity()
	ent.AddComponent(comp_model)
	ent.AddComponent(comp_trans)

	return ent

def createGun():
	comp_model = _engine.CreateComponentModel("resource/models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX")
	comp_model.SetAlbedoPath("resource/models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga")
	comp_model.SetNormalPath("resource/models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga")
	comp_model.SetMetalicPath("resource/models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga")
	comp_model.SetRouphnessPath("resource/models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga")
	comp_model.SetAOPath("resource/models/Cerberus_by_Andrew_Maximov/Textures/Raw/Cerberus_AO.tga")

	comp_trans = _engine.CreateComponentTransform()
	comp_trans.SetPosition([0.0, 0.0, -15.0])
	comp_trans.SetScale([0.1, 0.1, 0.1])
	comp_trans.SetRotationEular([1.600, 4.660, 0.1])

	ent = _engine.CreateEntity()
	ent.AddComponent(comp_model)
	ent.AddComponent(comp_trans)

	return ent

def createBackpack():
	comp_model = _engine.CreateComponentModel("resource/models/backpack/backpack.obj")
	comp_model.SetAlbedoPath("resource/models/backpack/Scene_-_Root_baseColor.jpeg")
	comp_model.SetNormalPath("resource/models/backpack/Scene_-_Root_normal.png")
	comp_model.SetMetalicPath("resource/images/pbr/rusted_iron/metallic.png")
	comp_model.SetRouphnessPath("resource/images/pbr/rusted_iron/roughness.png")
	comp_model.SetAOPath("resource/images/pbr/rusted_iron/ao.png")

	comp_trans = _engine.CreateComponentTransform()
	comp_trans.SetPosition([5.0, 0.0, -15.0])
	# comp_trans.SetScale([0.01, 0.01, 0.01])
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
		self.add_system(_engine.CreateSystemCamera())
		self.add_system(_engine.CreateSystemModel())
		self.add_system(_engine.CreateSystemInput())
		self.add_system(_engine.CreateSystemSyncRender())
		self.add_system(rotate_system.RotateSystem())

		cam_ent = CreateCamera()
		self.AddEntity(cam_ent)
		self.SetActiveCamera(cam_ent.get_id())

		# self.AddEntity(createGun())
		# self.AddEntity(createBackpack())
		self.AddEntity(createBall("rusted_iron", [-5.0, 0.0, -15.0]))
		self.AddEntity(createBall("gold", [-3.0, 0.0, -15.0]))
		self.AddEntity(createBall("grass", [-1.0, 0.0, -15.0]))
		self.AddEntity(createBall("plastic", [1.0, 0.0, -15.0]))
		self.AddEntity(createBall("wall", [3.0, 0.0, -15.0]))

		self.AddEntity(createPointLight([100.0, 100.0, 100.0], [-8.0, -2.0, -20.0]))
		self.AddEntity(createPointLight([100.0, 100.0, 100.0], [-2.0, 2.0, -20.0]))
		self.AddEntity(createPointLight([100.0, 100.0, 100.0], [8.0, -2.0, -20.0]))
		self.AddEntity(createPointLight([100.0, 100.0, 100.0], [2.0, 2.0, -20.0]))

	def add_system(self, sys):
		super().AddSystem(sys)
		self._sys_list.append(sys)
		sys.start()

