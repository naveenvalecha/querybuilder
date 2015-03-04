<?php

/**
 * @file
 * querybuilder_optimise.inc
 *
 * Optimise database.
 */

// Load querybuilder_common file.
module_load_include('inc', 'querybuilder', 'includes/querybuilder_common');

/**
 * Implements querybuilder_truncate
 *   A form for truncate query.
 *
 * @param $form
 * @param $form_state
 *
 * @return array
 */
function querybuilder_optimise($form, &$form_state) {
	$form['optimise'] = array(
	 '#type' => 'checkbox',
	 '#title' => t('Optimise database'),	
	 '#description' => t('When checked, optimise database every time when cron run.'),	
	);
 
 	$form['submit'] = array(
	 '#type' => 'submit',
	 '#value' => t('Save'),
	);
  return $form;
}

/**
 * Submit handler for 'add entry' form.
 */
function querybuilder_optimise_submit($form, $form_state) {
  $optimise = $form_state['values']['optimise'];
  if ($optimise) {
  	variable_set['clear_cache_by_cron'] = TRUE;
  }
 
	return;
}

/**
 * Implements hook_cron().
 */
function querybuilder_cron() {
  $clear_cache = variable_get['clear_cache_by_cron'];
  if (!empty($clear_cache)) {
  	drupal_flush_all_caches();
  	variable_set['clear_cache_by_cron'] = TRUE;
  }
}
