<div id="<?php print $block_html_id; ?>" class="panel-body <?php print $classes; ?>" <?php print isset($attributes) ? $attributes : ''; ?>>
    <div class="panel panel-sport panel-sport-all">
        <div class="panel-heading">
            <span class="tssporticon-big tssporticon-s<?=$aid?>-big"></span><h2><?=$title; ?></h2>
        </div>
    </div>
	<div class="block-content">
		<?=$countriesHtml;?>
	</div>
</div>