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
      'link_validations' => array(
        '#type' => 'item',
        '#markup' => '<ul><li>' .
          l(t('Validate Objects'), 'admin/islandora/tools/islandora_object_validation/validate_objects') .
          ' <span>- validate specific objects now.</span></li></ul>',
      ),
      'solr_limit_wrapper' => array(
        '#type' => 'fieldset',
        'solr_limit' => array(
          '#type' => 'textfield',
          '#title' => 'Solr query limit',
          '#description' => 'Use to control the maximum number of Solr records returned for "Solr query", "Collection", "Model" fetch methods.',
          '#default_value' => variable_get('islandora_object_validation_solr_limit', 10000),
        ),
        'submit1' => array(
          '#type' => 'submit',
          '#value' => t('Save Configuration'),
          '#weight' => 2,
        ),
      ),
    ),
    'validate_objects_wrapper' => array(
      '#type' => 'fieldset',
      '#attributes' => array(
        'class' => array('round_fieldset'),
      ),
      '#title' => t('Automatically validate objects?'),
      '#description' => t('View results of object validation testing.'),
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
      ),
    ),
    'content_model_wrapper' => array(
      '#type' => 'fieldset',
      '#attributes' => array(
        'class' => array('round_fieldset'),
      ),
      '#title' => t('Configure which content models are exposed to custom aliases'),
      '#description' => t('Select which datastreams are required for each Islandora '.
          'model.  <span class="required">Checkboxes</span> with a red border ' .
          'are considered to be "REQUIRED" by the Islandora solution pack related ' .
          'to that model.  Datastreams that are marked with N/A are disabled and ' .
          'should never occur for that object model.'),
      'submit3' => array(
        '#type' => 'submit',
        '#value' => t('Save Configuration'),
        '#prefix' => $validations_table,
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
  $skip_POST_names = array('op', 'form_build_id', 'form_token', 'form_id');
  $set_values = array();
  foreach ($_POST as $name => $value) {
    if (array_search($name, $skip_POST_names) === FALSE) {
      $set_values[$name] = $value;
    }
  }
  variable_set('islandora_object_validation_selected_cmodels', $set_values);
  variable_set('islandora_object_validation_solr_limit', $form_state['values']['solr_limit']);
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