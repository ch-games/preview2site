    <div id="left-column" class="col-xs-5">
        <?php if ($menu_block): ?><div class="nav-left-col block block-menu nav-info"><?=render($menu_block); ?></div><?php endif; ?>
    </div>

    <div id="content" class="col-xs-19">
        <div id="betting-center-page" class="panel panel-sport">            
            <?php if($title):?><div class="panel-heading"><?=$link_back?><h2><?=$title?></h2></div><?php endif;?>
            <div class="panel-body">              
                <?=$content; ?>
            </div>
            <?php if($map_size):?>
                <div id="map-location" style="height: <?=$map_size?>px;"><div id="map" style="width: 100%; height: 100%"></div></div>
            <?php endif;?>
        </div>
    </div> 
