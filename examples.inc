<?php

global $examples, $example_options;

if (!function_exists('t')) {
	function t($str) { return $str; }
}

# ------------------------------------------------------------------------------------ #

$example_names['todo'] = t('TODO - an example');
$example_jobs['todo'] = 1;
$examples['todo'] = <<<BOA
# TODO an example program here

BOA;

# ------------------------------------------------------------------------------------ #

$example_options = array(
	0 => t('-- Select Example --'),
	'TODO category' => array(
		'todo' => $example_names['todo'],
	),
);

?>
