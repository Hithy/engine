import _engine
import random
from ecs import hit_object_system
from ecs import rotate_system
from ecs import cdef

def createDirectionLight(color, direction, enable_shadow=False):
    comp_light = _engine.ComponentLight(cdef.LightType_Direction)
    comp_light.SetLightColor(color)
    comp_light.SetEnableShadow(enable_shadow)

    comp_trans = _engine.CreateComponentTransform()
    comp_trans.SetForward(direction)

    ent = _engine.CreateEntity()
    ent.AddComponent(comp_light)
    ent.AddComponent(comp_trans)

    return ent

def createPointLight(color, pos, radius, enable_shadow=False):
    comp_light = _engine.ComponentLight(cdef.LightType_Point)
    comp_light.SetLightColor(color)
    comp_light.SetEnableShadow(enable_shadow)
    comp_light.SetRadius(radius)

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

    comp_physics = _engine.ComponentPhysics(False, 0, [size * 2.0] * 3)

    comp_trans = _engine.CreateComponentTransform()
    comp_trans.SetPosition(pos)
    comp_trans.SetScale([size, size, size])
    # comp_trans.SetRotationEular([1.600, 4.660, 0.1])

    ent = _engine.CreateEntity()
    ent.AddComponent(comp_model)
    ent.AddComponent(comp_trans)
    ent.AddComponent(comp_physics)

    return ent

def createBox(material, pos, scale, rotate=None):
    comp_model = _engine.CreateComponentModel("resource/models/box/box.obj")
    if material:
        comp_model.SetAlbedoPath(material)

    comp_physics = _engine.ComponentPhysics(False, 3, [x / 2.0 for x in scale])
    comp_physics.SetKinematic(True)

    comp_trans = _engine.CreateComponentTransform()
    comp_trans.SetPosition(pos)
    comp_trans.SetScale(scale)
    if rotate:
        comp_trans.SetRotationEular(rotate)

    ent = _engine.CreateEntity()
    ent.AddComponent(comp_model)
    ent.AddComponent(comp_trans)
    ent.AddComponent(comp_physics)

    return ent

def createCylinder(material, pos, scale, rotate=None):
    comp_model = _engine.CreateComponentModel("resource/models/cylinder/cylinder.obj")
    if material:
        comp_model.SetAlbedoPath(material)

    # comp_physics = _engine.ComponentPhysics(False, 3, [x / 2.0 for x in scale])
    # comp_physics.SetKinematic(True)

    comp_trans = _engine.CreateComponentTransform()
    comp_trans.SetPosition(pos)
    comp_trans.SetScale(scale)
    if rotate:
        comp_trans.SetRotationEular(rotate)

    ent = _engine.CreateEntity()
    ent.AddComponent(comp_model)
    ent.AddComponent(comp_trans)
    # ent.AddComponent(comp_physics)

    return ent

def createBackpack(pos):
    comp_model = _engine.CreateComponentModel("resource/models/backpack/backpack.obj")
    # comp_model.SetAlbedoPath("resource/models/backpack/Scene_-_Root_baseColor.jpeg")
    comp_model.SetNormalPath("resource/models/backpack/Scene_-_Root_normal.png")

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
    # comp_cam.Lock(1)
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

    def SceneTAA(self):
        # enable IBL skybox
        # self.SetIBLPath("resource/images/Chelsea_Stairs/Chelsea_Stairs_3k.hdr")

        self.add_system(rotate_system.RotateSystem()) # rotate entity each frame

        # box
        self.AddEntity(createBox("", [-15.0, 0.0, -15.0], [2.0, 20.0, 2.0], [0, 0, 0.03]))
        self.AddEntity(createBox("", [-10.0, 0.0, -15.0], [2.0, 20.0, 2.0], [0, 0, 0.03]))
        self.AddEntity(createBox("", [-5.0, 0.0, -15.0], [2.0, 20.0, 2.0], [0, 0, 0.03]))
        self.AddEntity(createBox("", [0.0, 0.0, -15.0], [2.0, 20.0, 2.0], [0, 0, 0.03]))
        self.AddEntity(createBox("", [5.0, 0.0, -15.0], [2.0, 20.0, 2.0], [0, 0, 0.03]))
        self.AddEntity(createBox("", [10.0, 0.0, -15.0], [2.0, 20.0, 2.0], [0, 0, 0.03]))
        self.AddEntity(createBox("", [15.0, 0.0, -15.0], [2.0, 20.0, 2.0], [0, 0, 0.03]))
        self.AddEntity(createBox("", [20.0, 0.0, -15.0], [2.0, 20.0, 2.0], [0, 0, 0.03]))

        # light
        self.AddEntity(createDirectionLight([1.0, 1.0, 1.0], [1.0, -1.0, -1.0], 1))
        # self.AddEntity(createPointLight([10.0, 10.0, 10.0], [-5.0, 4.5, -10.0], 30.0, 1))
        # self.AddEntity(createPointLight([100.0, 100.0, 100.0], [-5.0, -2.5, -10.0], 1))

    def Scene1000(self):
        # self.add_system(hit_object_system.HitObjectSystem()) # rotate entity each frame

        # enable IBL skybox
        self.SetIBLPath("resource/images/Chelsea_Stairs/Chelsea_Stairs_3k.hdr")

        # floor
        self.AddEntity(createBox("", [-0.0, -5.0, -10.0], [50.0, 1.0, 50.0]))

        # wall
        self.AddEntity(createBox("", [-25.0, -3.0, -10.0], [1.0, 4.0, 50.0]))
        self.AddEntity(createBox("", [25.0, -3.0, -10.0], [1.0, 4.0, 50.0]))
        self.AddEntity(createBox("", [0.0, -3.0, -35.0], [50.0, 4.0, 1.0]))
        self.AddEntity(createBox("", [0.0, -3.0, 15.0], [50.0, 4.0, 1.0]))

        # objects
        self.AddEntity(createBall("rusted_iron", [-7.0, 0.0, -10.0], 0.5))
        self.AddEntity(createBall("gold", [-4.0, 0.0, -10.0], 0.5))
        self.AddEntity(createBall("grass", [-1.0, 0.0, -10.0], 0.5))
        self.AddEntity(createBall("plastic", [2.0, 0.0, -10.0], 0.5))
        self.AddEntity(createBall("wall", [5.0, 0.0, -10.0], 0.5))
        # self.AddEntity(createBackpack([5.0, 1.0, -15.0]))

        self.AddEntity(createBall("plastic", [-7.0, 0.0, -10.0], 0.5))
        self.AddEntity(createBall("plastic", [-4.0, 0.0, -10.0], 0.5))
        self.AddEntity(createBall("plastic", [-1.0, 0.0, -10.0], 0.5))
        self.AddEntity(createBall("plastic", [2.0, 0.0, -10.0], 0.5))
        self.AddEntity(createBall("plastic", [5.0, 0.0, -10.0], 0.5))
        self.AddEntity(createBall("plastic", [-7.0, 0.0, -7.0], 0.5))
        self.AddEntity(createBall("plastic", [-4.0, 0.0, -7.0], 0.5))
        self.AddEntity(createBall("plastic", [-1.0, 0.0, -7.0], 0.5))
        self.AddEntity(createBall("plastic", [2.0, 0.0, -7.0], 0.5))
        self.AddEntity(createBall("plastic", [5.0, 0.0, -7.0], 0.5))
        self.AddEntity(createBall("plastic", [-7.0, 0.0, -13.0], 0.5))
        self.AddEntity(createBall("plastic", [-4.0, 0.0, -13.0], 0.5))
        self.AddEntity(createBall("plastic", [-1.0, 0.0, -13.0], 0.5))
        self.AddEntity(createBall("plastic", [2.0, 0.0, -13.0], 0.5))
        self.AddEntity(createBall("plastic", [5.0, 0.0, -13.0], 0.5))

        self.AddEntity(createCylinder("resource/images/pbr/wall/albedo.png", [-10.0, -3.0, -10.0], [1.0, 1.0, 1.0]))
        self.AddEntity(createCylinder("resource/images/pbr/wall/albedo.png", [-7.0, -3.0, -10.0], [1.0, 1.0, 1.0]))
        self.AddEntity(createCylinder("resource/images/pbr/wall/albedo.png", [-4.0, -3.0, -10.0], [1.0, 1.0, 1.0]))
        self.AddEntity(createCylinder("resource/images/pbr/wall/albedo.png", [-1.0, -3.0, -10.0], [1.0, 1.0, 1.0]))
        self.AddEntity(createCylinder("resource/images/pbr/wall/albedo.png", [2.0, -3.0, -10.0], [1.0, 1.0, 1.0]))
        self.AddEntity(createCylinder("resource/images/pbr/wall/albedo.png", [5.0, -3.0, -10.0], [1.0, 1.0, 1.0]))
        self.AddEntity(createCylinder("resource/images/pbr/wall/albedo.png", [8.0, -3.0, -10.0], [1.0, 1.0, 1.0]))

        # light
        self.AddEntity(createDirectionLight([1.0, 1.0, 1.0], [1.0, -1.0, 0.0], 1))
        # self.AddEntity(createPointLight([10.0, 10.0, 10.0], [-5.0, 4.5, -10.0], 30.0, 1))
        # self.AddEntity(createPointLight([100.0, 100.0, 100.0], [-5.0, -2.5, -10.0], 1))
        for i in range(1000):
            self.AddEntity(
                createPointLight(
                    [2.0 * x for x in [random.random(), random.random(), random.random()]],
                    [random.random() * 48.0 - 24.0,
                     -2.5,
                     random.random() * 48.0 - 34.0],
                    3.0
                )
            )

    def InitBaseSystem(self):
        # systems
        self.add_system(_engine.CreateSystemCamera())
        self.add_system(_engine.CreateSystemModel())
        self.add_system(_engine.CreateSystemInput())
        
        self.add_system(_engine.CreateSystemPhysics())

        # camera
        cam_ent = CreateCamera()
        self.AddEntity(cam_ent)
        self.SetActiveCamera(cam_ent.get_id())

    def InitRenderSystem(self):
        self.add_system(_engine.CreateSystemSyncRender())

    def Init(self):
        self.InitBaseSystem()
        self.Scene1000()
        # self.SceneTAA()
        self.InitRenderSystem()

    def add_system(self, sys):
        super().AddSystem(sys)
        self._sys_list.append(sys)
        sys.start()

