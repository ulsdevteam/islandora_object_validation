<?php

/**
 * @file
 * Admin callbacks for islandora_object_validation.
 */

/**
 * Admin form.
 */
function islandora_object_validation_admin_settings(array $form, array &$form_state) {
  module_load_include('module', 'islandora_object_validation', 'islandora_object_validation');
  module_load_include('inc', 'islandora_object_validation', 'includes/utilities');
  module_load_include('inc', 'islandora', 'includes/utilities');
  islandora_object_validation_check_and_deploy_forena_reports();
  $all_cmodels = islandora_get_content_models();
  foreach ($all_cmodels as $cmodel => $pid_label_arr) {
    $model_requirements = islandora_object_validation_object_model_datastream_requirements($cmodel);
    if (array_key_exists('required', $model_requirements) === FALSE) {
      $datastreams = ((array_key_exists('optional', $model_requirements) === FALSE) ? array() : $model_requirements['optional']);
    }
    elseif (array_key_exists('optional', $model_requirements) === FALSE) {
      $datastreams = ((array_key_exists('required', $model_requirements) === FALSE) ? array() : $model_requirements['required']);
    }
    else {
      $datastreams = array_merge($model_requirements['required'], $model_requirements['optional']);
    }
    $all_cmodels[$cmodel]['datastreams'] = $datastreams;
  }

  usort($all_cmodels, 'islandora_object_validation_sortby_label');
  $validations_table = islandora_object_validation_get_validations_table($all_cmodels);
  drupal_set_title(t('Islandora Object Validation'));
  $breadcrumb = array(
    l(t('Home'), '<front>'),
    l(t('Administration'), 'admin'),
    l(t('Islandora'), 'admin/islandora'),
    t('Islandora Object Validation')
  );
  drupal_set_breadcrumb($breadcrumb);

  $report_links = islandora_object_validation_strap_report_reports();

  $form = array(
    'reports_wrapper' => array(
      '#type' => 'fieldset',
      '#title' => t('Reports'),
      '#attributes' => array(
        'class' => array('round_fieldset'),
      ),
      'report_links' => array(
        '#type' => 'item',
        '#markup' => '<ul><li>' . $report_links . '</li></ul>',
      ),
    ),
    'utility_wrapper' => array(
      '#type' => 'fieldset',
      '#title' => t('Validate objects'),
      '#attributes' => array(
        'class' => array('round_fieldset'),
      ),
      'solr_limit' => array(
        '#type' => 'textfield',
        '#title' => t('Default Solr query limit'),
        '#description' => t('Controls the value used on the Validate Objects form.'),
        '#default_value' => variable_get('islandora_object_validation_solr_limit', 10000),
      ),
      'cron_limit' => array(
        '#type' => 'textfield',
        '#title' => t('CRON limit'),
        '#description' => t('Use to control the maximum number of items that are processed from the validation queue for each CRON cycle.'),
        '#default_value' => variable_get('islandora_object_validation_cron_limit', 4000),
      ),
      'submit1' => array(
        '#type' => 'submit',
        '#value' => t('Save Configuration'),
        '#weight' => 2,
        '#submit' => array('islandora_object_validation_admin_settings_submit'),
        '#suffix' => '<br><br>' . l(t('Validate Objects'),
          'admin/islandora/tools/islandora_object_validation/validate_objects') .
          ' <span>- validate specific objects now.</span>',
      ),
    ),
    'validate_objects_wrapper' => array(
      '#type' => 'fieldset',
      '#title' => t('View results of object validation testing.'),
      '#attributes' => array(
        'class' => array('round_fieldset'),
      ),
      '#title' => t('Automatically validate objects?'),
      'validate_on_ingest' => array(
        '#type' => 'checkbox',
        '#title' => t('Validate objects automatically upon ingest.'),
        '#description' => t('When checked, this uses the Islandora "object ingested" hook to ' .
            'validate each object as it is ingested.'),
        '#default_value' => variable_get('islandora_object_validation_validate_on_ingest', 0),
      ),
      'submit2' => array(
        '#type' => 'submit',
        '#value' => t('Save Configuration'),
        '#weight' => 2,
        '#submit' => array('islandora_object_validation_admin_settings_submit'),
      ),
    ),
    'content_model_wrapper' => array(
      '#type' => 'fieldset',
      '#attributes' => array(
        'class' => array('round_fieldset'),
      ),
      '#title' => t('Islandora model requirements table'),
      '#description' => t('<em>Configure which datastreams need to exist for ' .
          'each content model that may be validated</em><br>Select which ' .
          'datastreams are required for each Islandora '.
          'model.  Checkboxes that are twice as large as the rest are considered ' .
          'to be "REQUIRED" by the Islandora solution pack related to that model.  ' .
          'Datastreams that are marked with N/A are disabled and should never ' .
          'occur for that object model, but this is not the case for aggregated ' .
          'PDF and consolidated OCR for paged content.<b>Note:</b> any ' .
          'datastream that is less than 1 byte in size will fail validation.'),
      'submit3' => array(
        '#type' => 'submit',
        '#value' => t('Save Configuration'),
        '#prefix' => $validations_table,
        '#submit' => array('islandora_object_validation_admin_settings_submit'),
        '#weight' => 10,
      ),
    ),
  );
  return $form;
}

/**
 * Submit handler for admin form.
 */
function islandora_object_validation_admin_settings_submit($form, &$form_state) {
  $skip_POST_names = array('op', 'form_build_id', 'form_token', 'form_id',
      'validate_on_ingest', 'cron_limit', 'solr_limit');
  $set_values = array();
  $relationship_values = array();
  foreach ($_POST as $name => $value) {
    if (array_search($name, $skip_POST_names) === FALSE) {
      // put the selectbox values into their own array of values
      if (strstr($name, '_relationship') || strstr($name, '_howmany') || strstr($name, '_related_to_model')) {
        @list($obj_pid_normalized, $verb) = explode("_", $name, 2);
        if ($obj_pid_normalized) {
          $relationship_values[$obj_pid_normalized][$verb] = $value;
        }
      }
      else {
        // put the value into the $set_values for cmodels
        $set_values[$name] = $value;
      }
    }
  }
  variable_set('islandora_object_validation_selected_cmodels', $set_values);
  variable_set('islandora_object_validation_model_relationships', $relationship_values);
  variable_set('islandora_object_validation_solr_limit', $form_state['values']['solr_limit']);
  variable_set('islandora_object_validation_cron_limit', $form_state['values']['cron_limit']);
  variable_set('islandora_object_validation_validate_on_ingest', $form_state['values']['validate_on_ingest']);
}

/**
 * Callback user sort function (usort) for two array items -- to be compared
 * by comparing the ['label'] value of each.
 *
 * @param array $a
 *   First item to compare.
 * @param array $b
 *   Second item to compare.
 * @return int
 *   Whether or not the two items are in the correctly sorted order.
 */
function islandora_object_validation_sortby_label($a, $b) {
  if (array_key_exists('label', $a) && array_key_exists('label', $b)) {
    if ($a['label'] == $b['label']) {
        return 0;
    }
    return ($a['label'] < $b['label']) ? -1 : 1;
  }
}
