<!DOCTYPE html>
<html lang="en-US">

	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=0.5">
		<link rel="stylesheet" href="css/style.css">

		<!-- <script src="/javascript/jquery/jquery.js"></script> -->
		<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js"></script>
		<script src="js/script.js"></script>
	</head>



	<div>
	<!-- Rounded switch -->
	SendMail enable
	<label class="switch">
	  <input id="checkBoxMail" type="checkbox" onclick="checkBoxMailClicked()" 
<?php
	if (file_exists("sendMailEnable.txt")) {
		echo ("checked");
	}
?>
>
	  <span class="slider round"></span>
	</label>
	</div>


	<body>

    <?php

	if (0 != filesize("log.txt")) {
		echo ("<object width=\"1700\" height=\"100\" type=\"text/plain\" data=\"log.txt\" border=\"0\" >
		</object>
		<br>");
	}

    // echo file_get_contents('log.txt');


    // include('log.txt');
    // exec("cat /var/log/apache2/error.log | grep auth_basic:error");

    // $user_agent = $_SERVER['HTTP_USER_AGENT'];
    // $os_array     = array(
    //     '/windows nt 10/i'      =>  'Windows 10',
    //     '/windows nt 6.3/i'     =>  'Windows 8.1',
    //     '/windows nt 6.2/i'     =>  'Windows 8',
    //     '/windows nt 6.1/i'     =>  'Windows 7',
    //     '/windows nt 6.0/i'     =>  'Windows Vista',
    //     '/windows nt 5.2/i'     =>  'Windows Server 2003/XP x64',
    //     '/windows nt 5.1/i'     =>  'Windows XP',
    //     '/windows xp/i'         =>  'Windows XP',
    //     '/windows nt 5.0/i'     =>  'Windows 2000',
    //     '/windows me/i'         =>  'Windows ME',
    //     '/win98/i'              =>  'Windows 98',
    //     '/win95/i'              =>  'Windows 95',
    //     '/win16/i'              =>  'Windows 3.11',
    //     '/macintosh|mac os x/i' =>  'Mac OS X',
    //     '/mac_powerpc/i'        =>  'Mac OS 9',
    //     '/linux/i'              =>  'Linux',
    //     '/ubuntu/i'             =>  'Ubuntu',
    //     '/iphone/i'             =>  'iPhone',
    //     '/ipod/i'               =>  'iPod',
    //     '/ipad/i'               =>  'iPad',
    //     '/android/i'            =>  'Android',
    //     '/blackberry/i'         =>  'BlackBerry',
    //     '/webos/i'              =>  'Mobile'
    // );
    // foreach ($os_array as $regex => $value)
    //     if (preg_match($regex, $user_agent))
    // 	$os_platform = $value;

	if (isset($_GET['lite'])) {
		$lite=(string)$_GET['lite'];
	} else {
		$lite=true;
	}

	include("showDay.php");

	$currentTime = time();
	$year = date('Y', $currentTime);
	$month= date('m', $currentTime);
	$day = date('d', $currentTime);
	showDay($year, $month, $day, $lite);


	$yesterday = $currentTime - 60 * 60 * 24;
	// $yesterday = $currentTime;
	$year = date('Y', $yesterday);
	$month= date('m', $yesterday);
	$day = date('d', $yesterday);
	showDay($year, $month, $day, $lite);




    ?>
	</body>
</html>
