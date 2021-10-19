import _engine
import time

class RotateSystem(_engine.System):
	def __init__(self):
		super().__init__(4)

		self._last_time = time.time()

	def tick(self, dt):
		curr_time = time.time()
		if curr_time - self._last_time < 1.0:
			return
		self._last_time = curr_time

		scn = self.GetScene()
		ents = scn.GetEntitiesByType(2)

		for ent in ents:
			comp_trans = ent.GetComponent(2)
			print("ent pos: ", str(comp_trans.GetPosition()))
