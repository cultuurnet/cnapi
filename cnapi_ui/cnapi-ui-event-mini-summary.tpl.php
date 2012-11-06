<div class="image">

  <?php if ($thumbnail) : ?>
    <?php print $thumbnail ?>
  <?php endif; ?>

</div>

<?php print render($title_prefix); ?>
<h<?php print $heading_level ?><?php print $title_attributes; ?>>
  <a href="<?php print $object_url; ?>"><?php print $title; ?></a>
</h<?php print $heading_level ?>>
<?php print render($title_suffix); ?>

<?php if ($when) : ?>
  <div class="when"><?php print $when ?></div>
<?php endif; ?>

<?php if ($where) : ?>
  <div class="where">
    <span class="where-location"><?php print $location ?></span>
    <span class="where-city"><?php print $city ?></span>
  </div>
<?php endif; ?>

<div class="more">
  <?php print $more_link ?>
</div>

<div class="actions">
  <?php if ($like_link) : ?><div class="like"><?php print $like_link ?></div><?php endif; ?>
  <?php if ($remove_link) : ?><div class="remove"><?php print $remove_link ?></div><?php endif; ?>
</div>

<div class="clearfix"></div>