<?php

/**
 * @file
 * Admin callbacks for islandora_object_validation.
 */

/**
 * Admin form.
 */
function islandora_object_validation_validate_objects(array $form, array &$form_state) {
  drupal_set_title(t('Validate objects'));
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Administration'), 'admin'),
    l(t('Islandora'), 'admin/islandora'),
    l(t('Islandora Object Validation'), 'admin/islandora/tools/islandora_object_validation'),
    t('Validate objects')
  );
  drupal_set_breadcrumb($breadcrumb);
  module_load_include('module', 'islandora_object_validation', 'islandora_object_validation');

  $report_table = '<div><b>Report results</b><i>There are no matching records.</i></div>';
  $form = array(
    'islandora_pid' => array(
      '#type' => 'textfield',
      '#title' => t('Islandora PID'),
      '#default_value' => '',
    ),
    'clearfilter' => array(
      '#type' => 'submit',
      '#value' => t('Clear'),
      '#weight' => 10,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Filter results'),
      '#weight' => 10,
      '#suffix' => $report_table
    ),
  );

  return $form;
}

/**
 * Submit handler for admin form.
 */
function islandora_object_validation_validate_objects_submit($form, &$form_state) {
  //
}

