<?php 
    //droping {anytext} marking
    $title = preg_replace('|{(?:[^}]*)}|', '', $title);
    //$title = preg_replace('|\((.*)\)|', '<span class="scope">$1</span>', $title);
?>
<div class="offer">
    <h4 class="odds-gametype"><?=$title?></h4>
    <div class="offers_list-odds">
		<table class="odds table-striped">
			<?=$eventsHtml; ?>
		</table>		
	</div>
</div>