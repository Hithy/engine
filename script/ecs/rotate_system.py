import _engine
import _math
import time
import random
from ecs import cdef

def RandomForce():
	base_froce = [random.random() - 0.5, 0.0, random.random() - 0.5]
	return [x * 5000.0 for x in base_froce]

class RotateSystem(_engine.System):
	def __init__(self):
		super().__init__(cdef.SystemType_Rotate)
		self._last_tick_time = time.time()

	def tick(self, dt):
		curr_time = time.time()
		if curr_time - self._last_tick_time < 1.0:
			return
		self._last_tick_time = curr_time

		world_obj = _engine.get_world()
		scn = self.GetScene()

		for ent in scn.GetEntitiesByType(cdef.ComponentType_Physics):
			comp_physics = ent.GetComponent(cdef.ComponentType_Physics)
			if not comp_physics.IsKinematic():
				if random.random() > 0.8:
					comp_physics.AddForce(RandomForce())
