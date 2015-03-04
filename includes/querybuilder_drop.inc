<?php

/**
 * @file
 * querybuilder_drop.inc
 *
 * Demonstrates for drop query.
 */

// Load querybuilder_common file.
module_load_include('inc', 'querybuilder', 'includes/querybuilder_common');

/**
 * Implements querybuilder_drop
 *   A form for drop query.
 *
 * @param $form
 * @param $form_state
 *
 * @return array
 */
function querybuilder_drop($form, &$form_state) {
  // Get list of tables name from database.
	$tbl_list = querybuilder_get_tables();
	$form['query_table'] = array(
    '#type' => 'select', 
    '#title' => 'Select Table',
    '#options' => $tbl_list,
    '#default_value' => '_none',
  );
 
	$form['build_query'] = array(
	 '#type' => 'checkbox',
	 '#title' => t('Build Query'),
		'#ajax' => array(
			'callback' => 'querybuilder_buildquery_callback',
			'wrapper' => 'textfields',
			'effect' => 'fade',
		),
	);
 
  $form['fieldsets'] = array(
   '#title' => t("Output"),
   '#prefix' => '<div id="textfields">',
   '#suffix' => '</div>',
   '#type' => 'fieldset',
  );

	if (!empty($form_state['values']['build_query']) && $form_state['values']['build_query']) {    
		$form['fieldsets']['query_outputs'] = array(
			'#title' => 'Query and Its output',
			'#markup' => querybuilder_drop_result($form, $form_state),
			'#prefix' => '<div id="query-output">',
			'#suffix' => '</div>',
		);
	}
  return $form;
}

/**
 * Implements querybuilder_insert_result
 *   Build query from user input.
 *
 * @param $form
 * @param $form_state
 *
 * @return string
 *   HTML string output
 */
function querybuilder_drop_result($form, $form_state) {
  $query_table = $form_state['values']['query_table'];
  if ($query_table != '_none') {	
		$act_query = '$transaction = db_transaction();';
		$act_query .= "<br>" . 'try {';
		$act_query .= "<br>" . '$query = db_drop_table(\'' . $query_table . '\');';	
		$act_query .= "<br>" . '}';
		$act_query .= "<br>" . 'catch (Exception $e) {';
		$act_query .= "<br>" . '$transaction->rollback();';
		$act_query .= "<br>" . 'watchdog_exception(\'Query_Issue\', $e);';
		$act_query .= "<br>" . '}';		
  }
  else {
		$act_query = 'Insufficient data!';
	}
	$header = array(array('data' => "Query"));
	$rows = array();
	$rows['query'] = array($act_query);
	$output = theme('table', array('header' => $header, 'rows' => $rows)); 
	return $output;
}

