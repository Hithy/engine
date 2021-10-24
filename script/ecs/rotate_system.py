import _engine
import _math
from ecs import cdef

class RotateSystem(_engine.System):
	def __init__(self):
		super().__init__(cdef.SystemType_Rotate)

	def tick(self, dt):
		world_obj = _engine.get_world()
		scn = self.GetScene()

		delta_rotate = _math.GenRotation([0.0, 1.0, 0.0], dt * 50.0);

		for ent in scn.GetEntitiesByType(cdef.ComponentType_Model):
			comp_trans = ent.GetComponent(cdef.ComponentType_Transform)
			old_rotate = comp_trans.GetRotation()
			new_rotate = _math.Rotate(old_rotate, delta_rotate)
			comp_trans.SetRotation(new_rotate)
