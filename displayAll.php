<!DOCTYPE html>
<html>

<body>

    <?php
    $dir = "motion/" . (string)$_GET['dir'];

    $files = scandir("$dir");

    foreach ($files as $file) {
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