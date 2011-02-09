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
	((descriptor->input_count == 0 && descriptor->inputs == NULL) ||
	 (descriptor->input_count != 0 && descriptor->inputs != NULL)) &&
	((descriptor->output_count == 0 && descriptor->outputs == NULL) ||
	 (descriptor->output_count != 0 && descriptor->outputs != NULL)) &&
	((descriptor->internal_count == 0 && descriptor->internals == NULL) ||
	 (descriptor->internal_count != 0 && descriptor->internals != NULL)))) {
    return false;
  }

  size_t idx;
  size_t idx2;

  /* Inputs must all be different. */
  if (descriptor->input_count > 0) {
    for (idx = 0; idx < descriptor->input_count - 1; ++idx) {
      for (idx2 = idx + 1; idx2 < descriptor->input_count; ++idx2) {
	if (descriptor->inputs[idx] == descriptor->inputs[idx2]) {
	  return false;
	}
      }
    }
  }

  /* Outputs must all be different. */
  if (descriptor->output_count > 0) {
    for (idx = 0; idx < descriptor->output_count - 1; ++idx) {
      for (idx2 = idx + 1; idx2 < descriptor->output_count; ++idx2) {
	if (descriptor->outputs[idx] == descriptor->outputs[idx2]) {
	  return false;
	}
      }
    }
  }

  /* Internals must all be different. */
  if (descriptor->internal_count > 0) {
    for (idx = 0; idx < descriptor->internal_count - 1; ++idx) {
      for (idx2 = idx + 1; idx2 < descriptor->internal_count; ++idx2) {
	if (descriptor->internals[idx] == descriptor->internals[idx2]) {
	  return false;
	}
      }
    }
  }

  /* System input cannot be in inputs. */
  for (idx = 0; idx < descriptor->input_count; ++idx) {
    if (descriptor->system_input == descriptor->inputs[idx]) {
      return false;
    }
  }

  /* System output cannot be in outputs. */
  for (idx = 0; idx < descriptor->output_count; ++idx) {
    if (descriptor->system_output == descriptor->outputs[idx]) {
      return false;
    }
  }

  return true;
}

