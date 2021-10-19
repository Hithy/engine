import _engine
# import numpy
from ecs import rotate_system

def createGun():
	comp_model = _engine.CreateComponentModel("resource/models/ball/ball.obj")
	comp_model.SetModelPath("resource/models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX")
	comp_model.SetAlbedoPath("resource/models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga")
	comp_model.SetNormalPath("resource/models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga")
	comp_model.SetMetalicPath("resource/models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga")
	comp_model.SetRouphnessPath("resource/models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga")
	comp_model.SetAOPath("resource/models/Cerberus_by_Andrew_Maximov/Textures/Raw/Cerberus_AO.tga")

	comp_trans = _engine.CreateComponentTransform()
	comp_trans.SetPosition([0.0, 5.0, -15.0])
	comp_trans.SetScale([0.1, 0.1, 0.1])
	comp_trans.SetRotationEular([1.600, 4.660, 0.1])

	ent = _engine.CreateEntity()
	ent.AddComponent(comp_model)
	ent.AddComponent(comp_trans)

	return ent

def createBall():
	comp_model = _engine.CreateComponentModel("resource/models/ball/ball.obj")
	comp_trans = _engine.CreateComponentTransform()
	comp_trans.SetPosition([-5.0, 0.0, -15.0])
	# comp_trans.SetScale([0.01, 0.01, 0.01])

	backpack_ent = _engine.CreateEntity()
	backpack_ent.AddComponent(comp_model)
	backpack_ent.AddComponent(comp_trans)

	return backpack_ent

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
		self.AddEntity(createBall())

	def add_system(self, sys):
		super().AddSystem(sys)
		self._sys_list.append(sys)
		sys.start()

