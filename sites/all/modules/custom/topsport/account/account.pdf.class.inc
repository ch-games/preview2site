<?php

	require_once(DRUPAL_ROOT . '/sites/all/libraries/tcpdf/tcpdf.php');
	require_once(DRUPAL_ROOT . '/sites/all/libraries/fpdi/fpdi.php');

class AccountPDF extends FPDI { function Header(){}  function Footer(){} }

?>