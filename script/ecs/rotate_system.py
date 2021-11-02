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

    def tick(self, dt):
        world_obj = _engine.get_world()
        scn = self.GetScene()

        delta_x = dt * 1.0

        for ent in scn.GetEntitiesByType(cdef.ComponentType_Model):
            comp_trans = ent.GetComponent(cdef.ComponentType_Transform)
            if comp_trans:
                curr_pos = comp_trans.GetPosition()
                curr_pos[0] += delta_x
                if curr_pos[0] > 20:
                    curr_pos[0] -= 40
                comp_trans.SetPosition(curr_pos)
