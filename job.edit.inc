<?php

function boa_job_edit($jobId) {
	drupal_goto('boa/query', array('query' => array('edit' => $jobId)));
	drupal_exit();
}

?>
