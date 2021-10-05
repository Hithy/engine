import _engine
import time

next_print_time = 0

def tick():
	global next_print_time
	curr_time = int(time.time() * 1000)

	if curr_time > next_print_time:
		next_print_time = curr_time + 1000
		
		world_obj = _engine.get_world()
		scene_obj = world_obj.get_active_scene()
		# entities = scene_obj.get_entity_list()
		entities = _engine.get_scene_entities(scene_obj)
		print("python tick, entities: " + str(entities) + " time: " + str(curr_time))

def __start__():
	print("python initing...")

	_engine.print_list([8,7,6,5,4,3])

	print("1 + 2 = " + str(_engine.test_add(1, 2)))

	print("python inited")
	