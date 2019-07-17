<?php

/**
 * @file
 * Admin callbacks for islandora_object_validation.
 */

/**
 * Admin form.
 */
function islandora_object_validation_history_form(array $form, array &$form_state, AbstractObject $object) {
  module_load_include('module', 'islandora_object_validation', 'islandora_object_validation');
  module_load_include('inc', 'islandora_object_validation', 'includes/utilities');

  $history_table = islandora_object_validation_get_history_table($object->id);

  $form['history_form'] = array(
    'objectsformset' => array(
      '#type' => 'fieldset',
      '#title' => t('Validation History'),
      'the_table' => array(
        '#type' => 'item',
        '#markup' => $history_table,
      ),
    ),
  );

  return $form;
}

/**
 * Submit handler for admin form.
 */
function islandora_object_validation_validate_objects_submit($form, &$form_state) {
  module_load_include('inc', 'islandora_object_validation', 'includes/utilities');

}
