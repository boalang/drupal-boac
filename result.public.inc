<?php

function boa_result_public($jobId) {
	if (!isValidJob($jobId))
		invalidJob($jobId);

	$query = db_update('boa_jobs');
	$query->fields(array('public' => 1));
	$query->condition('id', $jobId);
	$query->execute();

	drupal_set_message(t('The result is now PUBLIC and visible to anyone.'), 'warning');
	drupal_goto("boa/result/$jobId");
	drupal_exit();
}

function boa_result_private($jobId) {
	if (!isValidJob($jobId))
		invalidJob($jobId);

	$query = db_update('boa_jobs');
	$query->fields(array('public' => 0));
	$query->condition('id', $jobId);
	$query->execute();

	drupal_set_message(t('The result is now PRIVATE and visible to only you.'), 'warning');
	drupal_goto("boa/result/$jobId");
	drupal_exit();
}

?>
