<?php

/**
 * Implements function for fetching list of table name from database.
 *
 * @return array $tables_list
 *   Returns array of all tables name.
 */
function querybuilder_get_tables() {
	global $databases;
  $db_name = $databases['default']['default']['database'];
  // Fetch list of table name.
  $tables_list = db_query("SHOW tables FROM " . $db_name, array())->fetchAllKeyed(0,0);  
  $tables_list['_none'] = "Select table";
	return $tables_list;
}

/**
 * Implements helper function for fetching list of fields name from table(s).
 *
 * @param string/array $tables
 *   Array of tables name.
 *
 * @return array $field_list
 *   Returns array of all fields name.
 */
function _querybuilder_get_fields($tables) {
	if (!is_array($tables) && !empty($tables) && ($tables != '_none')) {
		$result = db_query("SHOW COLUMNS FROM " . $tables, array())->fetchAll();
		foreach ($result as $record) {
			$field_list[$record->Type] = $record->Field;
		}
		return $field_list;
  }
  else {
		if (($tables != '_none') && (is_array($tables))) { 
			if (!array_key_exists('_none', $tables)) {
				foreach ($tables as $key => $value) { 
				// Fetch list of column names from table(s).
					$result = db_query("SHOW COLUMNS FROM " . $value, array())->fetchAll();
					foreach ($result as $record) {
						$field_list[$value . '.' . $record->Type] = $value . '.' .$record->Field;
					}
				}
      }				
			return $field_list;
		} 
	}
}

/**
 * Implements function for validate data type.
 *
 * @param string $data_type
 *   Data type name.
 * @param int $length
 *   Length of data type.
 * @param string $field_value
 *   Value of data type.
 *
 * @return boolean 
 *   Returns TRUE/FALSE.
 */
function validate_value($data_type, $length, $field_value) {
  if (empty($field_value)) {
		return FALSE;
	}
	$result = FALSE;
  switch ($data_type) {
    case 'varchar':
      $len = strlen($field_value);
      if ($len > $length) {
        drupal_set_message("Length of text exceeded.", 'warning');
      }
      $result = TRUE;
      break;
		case 'longtext':
      $len = strlen($field_value);
			$length = 500;
      if ($len > $length) {
        drupal_set_message("Length of text exceeded.", 'warning');
      }
      $result = TRUE;
    break;
    case 'int':
      if (!is_numeric($field_value)) {       
        $result = FALSE;
      }
      else {
         $result = TRUE;
      }
      break;
    case 'tinyint':
       if (!is_numeric($field_value)) {       
        $result = FALSE;
      }
      else {
         $result = TRUE;
      }
      break;
     case 'bigint':
       if (!is_numeric($field_value)) {       
        $result = FALSE;
      }
      else {
         $result = TRUE;
      }
      break;
		case 'float':
      if (!is_numeric($field_value)) {       
        $result = FALSE;
      }
      else {
         $result = TRUE;
      }
      break;
  }
  return $result;
}

/**
 * Implements function for getting operator.
 *
 * @param string $operator
 *   Operator name.
 *
 * @return string $sign 
 *   Returns operator symbol.
 */
function actual_operator($operator) {
	switch ($operator) {
		case 'eq':
			$sign = '=';
			break;
		case 'neq':
			$sign = '<>';
			break;
		case 'sw':
			$sign = 'S_LIKE';
			break;
		case 'nsw':
			$sign = 'NOT S_LIKE';
			break;
		case 'ew':
			$sign = 'LIKE_E';
			break;
		case 'new':
			$sign = 'NOT LIKE_E';
			break;
		case 'gt':
			$sign = '>';
			break;
		case 'lt':
			$sign = '<';
			break;
	  case 'elt':
			$sign = '<=';
			break;
	  case 'egt':
			$sign = '>=';
			break;
		case 'in':
			$sign = 'IN';
			break;
		case 'nin':
			$sign = 'NOT IN';
			break;
		case 'em':
			$sign = 'isNull';
			break; 
		case 'nem':
			$sign = 'isNotNull';
			break; 
		default:
			$sign = '=';
			break;
	}
	return $sign;
}

/**
 * Implements callback function .
 *
 * @return renderable array
 *   According to form-ID return renderable array
 */
function querybuilder_querytable_callback($form, $form_state) {
  // Check form-ID.
	if ($form['form_id']['#value'] == "querybuilder_update") { 
      return array(
      '#type' => 'ajax',
      '#commands' => array(
        ajax_command_replace("#query-table-fields", render($form['query_fields'])),
        ajax_command_replace("#query-table-fields-condition", render($form['condition_fields'])),
      ),
     );
  }
  elseif ($form['form_id']['#value'] == "querybuilder_select") { 
      return array(
      '#type' => 'ajax',
      '#commands' => array(
        ajax_command_replace("#query-table-fields", render($form['query_fields'])),
        ajax_command_replace("#query-table-join", render($form['placeholder_join'])),
        ajax_command_replace("#query-table-fields-condition", render($form['condition_fields'])),
      ),
     );
  }
  elseif ($form['form_id']['#value'] == "querybuilder_merge") { 
      return array(
      '#type' => 'ajax',
      '#commands' => array(
        ajax_command_replace("#query-table-fields", render($form['query_fields'])),
        ajax_command_replace("#query-table-fields-update", render($form['query_fields_update'])),
        ajax_command_replace("#query-table-fields-key", render($form['query_fields_key'])),
      ),
     );
  }
  else {
    return $form['query_fields'];
  }  
}

/**
 * Implements callback function .
 *
 * @return renderable array (the placeholder element)
 */
function querybuilder_queryfield_callback($form, $form_state) {
  return $form['placeholder'];
}

/**
 * Implements callback function .
 *
 * @return renderable array (the placeholder_update element)
 */
function querybuilder_queryfield_update_callback($form, $form_state) {
  return $form['placeholder_update'];
}

/**
 * Implements callback function .
 *
 * @return renderable array (the placeholder_cond element)
 */
function querybuilder_condfield_callback($form, $form_state) {
  return $form['placeholder_cond'];
}

/**
 * Implements callback function .
 *
 * @return renderable array (the fieldsets element)
 */
function querybuilder_buildquery_callback($form, $form_state) {
  return $form['fieldsets'];
}
