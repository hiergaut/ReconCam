<!DOCTYPE html>
<html>

<body>

    <?php
	$year=(string)$_GET['year'];
	$month=(string)$_GET['month'];
	$day=(string)$_GET['day'];
	$file=(string)$_GET['file'];

    $dir = "motion/$year/$month/$day/$file";

    // $dir = "motion/" . (string)$_GET['dir'];

    $files = scandir("$dir");

    // foreach ($files as $file) {
    foreach (array_reverse($files) as $file) {
        if (fnmatch("*.jpg", $file)) {
            echo "
                <a href=\"$dir/$file\" >
                    <img src=\"$dir/$file\" width=320 height=240 />
                </a>
                ";
        }

        // if (fnmatch("*.ogv", $file)) {
        //     echo "
        //         <video controls>
        //             <source src=\"$dir/$file\" type=video/ogg>
        //         </video>
        //         ";
        // }
    }

    ?>

</body>

</html>
