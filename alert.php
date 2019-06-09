<!DOCTYPE html>
<html>

<head>
    <!-- <script type="text/javascript" src="live.js"></script> -->
    <meta http-equiv="refresh" content="17">
    <!-- javascript -->
    <script>
        // function refreshPage() {
        //     window.location.reload();
        // }

        // var sound = new Howl({
        //     src: ['alert.mp3'],
        //     autoplay: true,
        //     loop: true
        // });
        // sound.play();
        // // var title
        // var song = new Audio;
        // song.src = 'alert.mp3';
        // song.play();
        // // audio.play();

        var audio = new Audio('alert.mp3');
        audio.load();

        const title = 'Simple title';
        const options = {
            // body: 'Simple body',
            image: 'alert.jpg',
            // icon: 'alert.jpg'
            // sound: "alert.mp3"
            // silent: "false"
        };

        function notifyMe(event) {
            audio.play();
            // Let's check if the browser supports notifications
            if (!("Notification" in window)) {
                alert("This browser does not support system notifications");
            }

            // Let's check whether notification permissions have already been granted
            else if (Notification.permission === "granted") {
                // If it's okay let's create a notification
                var notification = new Notification(event, options);
            }

            // Otherwise, we need to ask the user for permission
            else if (Notification.permission !== 'denied') {
                Notification.requestPermission(function(permission) {
                    // If the user accepts, let's create a notification
                    if (permission === "granted") {
                        var notification = new Notification("Hi there!");
                    }
                });
            }

            // Finally, if the user has denied notifications and you 
            // want to be respectful there is no need to bother them any more.
        }
        // notifyMe();
        // });
        // const title = 'Simple title';
        // const options = {
        //     body: 'Simple body'
        // };
        // registration.showNotification(title, options);

        // Notification.requestPermission().then(function(result) {
        //     console.log(result);
        // });
        // // sound.play();
    </script>
</head>

<!-- <audio>
    <source src="alert.mp3" type="audio/mpeg">
    <object>
        <param name="autostart" value="true">
        <param name="src" value="alert.mp3">
        <param name="autoplay" value="true">
        <param name="controller" value="false">
        <embed src="alert.mp3" controller="false" autoplay="true" autostart="true" type="audio/mpeg"></embed>
    </object>
</audio> -->

<body>

    <img src="alert.jpg" />

    <?php
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
    $dir = "alert";
    $files = scandir("$dir");
    foreach ($files as $file) {
        if ($file != '.' && $file != '..') {
            // echo "$file";
            // exec('rm -f alert/fuck');
            // unlink('/srv/http/ReconCam/alert/fuck');
            // echo "<script> let audio = new Audio('alert.mp3');
            // audio.play(); </script>";
            // rm('alert/fuck');
            echo "<script> notifyMe(\"$file\"); </script>";
            // unlink("$dir/$file");
        }
    }



    // $dir = "motion";
    // $files = scandir("$dir");


    // // --------------------------- TIMELAPSE
    // foreach ($files as $file) {
    //     // if (fnmatch("*.jpg", $file)) {
    //     if (fnmatch("timelapse_*", $file)) {
    //         // echo "
    //         //     <a href=\"$dir/$file\" >
    //         // 	<img src=\"$dir/$file\" width=320 height=240 />
    //         //     </a>
    //         //     ";
    //         // $cur_dir =exec("echo $file | cut -d. -f1");
    //         echo "
    // 	    <a href=\"displayAll.php?dir=$file\" >
    // 		    <img src=\"$dir/$file/timelapse.gif\" width=320 height=240 />
    // 	    </a>
    // 	    ";
    //     }
    // }

    // echo "<hr>";

    // foreach ($files as $file) {
    //     if (fnmatch("timelapse_*", $file)) {
    //         // if (fnmatch("*.gif", $file)) {
    //         echo "
    //         <a href=\"$dir/$file/latest.jpeg\" >
    //         <img src=\"$dir/$file/latest.jpeg\" width=320 height=240 />
    //         </a>
    //         ";
    //         // $cur_dir =exec("echo $file | cut -d. -f1");
    //         // echo "
    //         // 	    <a href=\"displayAll.php?dir=$cur_dir\" >
    //         // 		    <img src=\"$dir/$file\" width=320 height=240 />
    //         // 	    </a>
    //         // 	    ";
    //     }
    // }

    // // --------------------------- MOVEMENTS
    // $heure = 24;
    // $previousHeure = 24;
    // foreach (array_reverse($files) as $file) {
    //     if ($file != '.' && $file != '..') {
    //         if (!fnmatch("timelapse_*", $file)) {
    //             // echo "$file";
    //             // if (file_exists("$dir/$file/complete.var")) {
    //             $heure = exec("echo $file | cut -c1-2");
    //             #$fileName =exec("echo $file | awk -F. '{print $1}'");

    //             if ($heure != $previousHeure) {
    //                 $previousHeure = $heure;

    //                 echo "<hr>";
    //                 echo "<div align =center><font size=6pt>$heure h</font><br</div>";
    //                 echo "<hr>";
    //             }

    //             // $video =exec("echo $file | cut -d: -f2,3");
    //             // $video = "$dir/$file/clip.gif";
    //             // $video ="$video.mp4";

    //             #			echo "
    //             #			<video controls src=\"motion/$video\" poster=\"$file\" </video>
    //             #			";
    //             #				<source src=\"motion/$fileName.mp4\" type=\"video/mp4\" />
    //             #				<source src=\"motion/$fileName.avi\" type=\"video/avi\" />
    //             #			</video>
    //             #			";


    //             // $style = "";
    //             // $nbPhoto = exec("ls $dir/$file/cap_* | wc -l");
    //             // // echo "$nbPhoto";
    //             // if ($nbPhoto <= 4) {
    //             //     $style = "$style border-bottom:2px solid blue;";
    //             // }

    //             // if (file_exists("$dir/$file/nand.var")) {
    //             //     $style = "$style border-right:2px solid red;";
    //             // }

    //             // if (file_exists("$dir/$file/cap_1.jpg")) {
    //             //         if ($os_platform == "Android") {
    //             //             echo "
    //             // <a href=\"display.php?dir=$file\" >
    //             //     <img style=\"$style\" src=\"$dir/$file/diffMax.jpg\" width=320 height=240 />
    //             // </a>
    //             // ";
    //             //         } else {
    //             //     echo "
    //             // <a href=\"$video\" >
    //             //     <img style=\"$style\" src=\"$dir/$file/trace.jpeg\" width=320 height=240 />
    //             // </a>
    //             // ";
    //             // if (file_exists("$dir/$file/trace.jpg")) {

    //             // echo "
    //             //     <a href=\"displayAll.php?dir=$file\" >
    //             //         <img src=\"$dir/$file/trace.jpg\" width=320 height=240 />
    //             //     </a>
    //             //     ";

    //             echo "
    //                 <a href=\"$dir/$file/video.webm\" >
    //                     <img src=\"$dir/$file/trace.jpg\" width=320 height=240 />
    //                 </a>
    //                 ";
    //             // }
    //             // }
    //             //         } else {
    //             //             echo "
    //             // <a href=\"$dir/$file/cap_0.jpg\" >
    //             //     <img src=\"$dir/$file/cap_0.jpg\" width=320 height=240 />
    //             // </a>
    //             // ";
    //         }
    //     }
    // }
    // }

    // echo "
    // <hr>
    // <button type=\"submit\"  onClick=\"refreshPage()\" style=\"width: 100%;\"><img src=\"refresh.png\" width=\"50\" height=\"50\" style=\"background-color:light-gray;\" /></button>
    // <hr>
    // ";


    ?>


</body>

</html>