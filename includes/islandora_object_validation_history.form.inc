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
    'validate_button' => array(
      '#type' => 'submit',
      '#value' => t('Validate this object now'),
      '#suffix' => '<br><br>',
    ),
    'pid' => array(
      '#type' => 'hidden',
      '#default_value' => $object->id,
    ),
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
function islandora_object_validation_history_form_submit($form, &$form_state) {
  $pid = $form_state['values']['pid'];
  $islandora_object = islandora_object_load($pid);
  if (is_object($islandora_object)) {
    module_load_include('inc', 'islandora_object_validation', 'includes/utilities');
    islandora_object_validation_validate($islandora_object);
    drupal_set_message(t('The object @pid has been validated.  Check the result below.',
        array('@pid' => $pid)), 'status');
  }
  else {
    drupal_set_message(t('The object @pid does not exist in islandora and cannot ' .
        'be validated.'), 'error');
  }
  drupal_goto('islandora/object/' . $pid . '/manage/islandora_object_validation');
}
