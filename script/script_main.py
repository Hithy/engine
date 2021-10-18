import _engine
import time
import math

next_print_time = 0
list_tick_time = 0
is_inited = False

entity_list = []

def createBall():
	comp_model = _engine.CreateComponentModel("resource/models/ball/ball.obj")
	comp_trans = _engine.CreateComponentTransform()
	comp_trans.SetPosition([0.0, 0.0, -15.0])
	# comp_trans.SetScale([0.01, 0.01, 0.01])

	backpack_ent = _engine.CreateEntity()
	backpack_ent.AddComponent(comp_model)
	backpack_ent.AddComponent(comp_trans)

	return backpack_ent

def doInit():
	global list_tick_time
	list_tick_time = time.time()

	world_obj = _engine.get_world()
	scene_obj = world_obj.get_active_scene()

	new_ent = createBall()
	scene_obj.AddEntity(new_ent)
	entity_list.append(new_ent.get_id())


def tick():
	global next_print_time
	global list_tick_time
	global is_inited
	if not is_inited:
		doInit();
		is_inited = True
	curr_time = time.time()
	dt = curr_time - list_tick_time

	x = math.sin(curr_time) * 10
	z = math.cos(curr_time) * 10

	world_obj = _engine.get_world()
	scene_obj = world_obj.get_active_scene()

	for ent_id in entity_list:
		ent = scene_obj.GetEntitiesById(ent_id)
		comp_trans = ent.GetComponent(2)
		comp_trans.SetPosition([x, 0.0, z])

	if curr_time > next_print_time:
		next_print_time = curr_time + 1

		for ent_id in entity_list:
			ent = scene_obj.GetEntitiesById(ent_id)
			comp_trans = ent.GetComponent(2)
			print ("curr pos: ", str(comp_trans.GetPosition()))

def __start__():
	print("python initing...")

	_engine.print_list([8,7,6,5,4,3])

	print("1 + 2 = " + str(_engine.test_add(1, 2)))

	print("python inited")
	