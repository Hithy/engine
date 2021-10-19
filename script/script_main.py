import _engine

import time
from ecs import scene

g_scene = None
g_last_tick_time = time.time()

def tick():
	global g_last_tick_time
	curr_time = time.time()
	dt = float(curr_time - g_last_tick_time)
	g_last_tick_time = curr_time

	g_scene.tick(dt)

def CreateScene():
	world_obj = _engine.get_world()
	scene_obj = scene.PyScene()
	world_obj.AddScene(scene_obj)
	return scene_obj

def __start__():
	print("python initing...")

	global g_scene
	g_scene = CreateScene()
	g_scene.Init()

	print("python inited")
	