#include "ueioa.h"

#include <assert.h>

bool
descriptor_check (descriptor_t* descriptor)
{
  assert (descriptor != NULL);
  /* Check that various things are not NULL. */
  if (!(descriptor->constructor != NULL &&
	descriptor->system_input != NULL &&
	descriptor->system_output != NULL &&
	descriptor->inputs != NULL &&
	descriptor->outputs != NULL &&
	descriptor->internals != NULL)) {
    return false;
  }

  size_t idx;
  size_t idx2;

  /* Inputs must all be different. */
  if (descriptor->inputs[0] != NULL) {
    for (idx = 0; descriptor->inputs[idx + 1] != NULL; ++idx) {
      for (idx2 = idx + 1; descriptor->inputs[idx2] != NULL; ++idx2) {
	if (descriptor->inputs[idx] == descriptor->inputs[idx2]) {
	  return false;
	}
      }
    }
  }

  /* Outputs must all be different. */
  if (descriptor->outputs[0] != NULL) {
    for (idx = 0; descriptor->outputs[idx + 1] != NULL; ++idx) {
      for (idx2 = idx + 1; descriptor->outputs[idx2] != NULL; ++idx2) {
	if (descriptor->outputs[idx] == descriptor->outputs[idx2]) {
	  return false;
	}
      }
    }
  }

  /* Internals must all be different. */
  if (descriptor->internals[0] != NULL) {
    for (idx = 0; descriptor->internals[idx + 1] != NULL; ++idx) {
      for (idx2 = idx + 1; descriptor->internals[idx2] != NULL; ++idx2) {
	if (descriptor->internals[idx] == descriptor->internals[idx2]) {
	  return false;
	}
      }
    }
  }

  /* System input cannot be in inputs. */
  for (idx = 0; descriptor->inputs[idx] != NULL; ++idx) {
    if (descriptor->system_input == descriptor->inputs[idx]) {
      return false;
    }
  }

  /* System output cannot be in outputs. */
  for (idx = 0; descriptor->outputs[idx] != NULL; ++idx) {
    if (descriptor->system_output == descriptor->outputs[idx]) {
      return false;
    }
  }

  return true;
}

