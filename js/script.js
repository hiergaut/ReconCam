

function enableMail(){
	$.ajax({
		type: 'POST',
		url: "enableMail.php",
		// data: "something",
		// success: function(result) {
			// alert('the data was successfully sent to the server');
		// }
	})
}
function disableMail(){
	$.ajax({
		type: 'POST',
		url: "disableMail.php",
		// data: "something",
		// success: function(result) {
			// alert('the data was successfully sent to the server');
		// }
	})
}

function checkBoxMailClicked() {
	var isChecked = document.getElementById("checkBoxMail").checked;
	const filename = 'sendMailEnable.txt';

	if(isChecked){
		console.log("Input is checked");

		// document.write("<?php $file = fopen(\"sendMailEnable.txt\"); fclose($file);?>");
		// alert(var2);
		// document.write('<?php echo fuck; ?>');
		enableMail();
		// $system('ls -l');


	} else {
		console.log("Input is NOT checked");

		disableMail();
	}
}
