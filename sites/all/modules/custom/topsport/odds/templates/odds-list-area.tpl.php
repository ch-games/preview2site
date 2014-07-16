<div id="<?php print $block_html_id; ?>" class="panel-body <?=$classes; ?>" <?=isset($attributes) ? $attributes : ''; ?>>
    <?php if(isset($show_header) && $show_header) :?>
    <div class="panel panel-sport panel-sport-all"<?php if($area_path):?> onclick="window.location='/<?=$area_path?>'" style="cursor: pointer;"<?php endif;?>>
        <div class="panel-heading"><span class="tssporticon-big tssporticon-s<?=$aid;?>-big"></span><h2><?=$title;?></h2></div>
    </div>
    <?php endif; ?>
	<div class="block-content">
		<?=$content;?>
	</div>
</div>