<?php

function theme_cnapi_ui_page_list_rss($variables) {
  $objects = $variables['objects'];
  $type = $variables['type'];

  $items = array();
  if ($objects) {
    foreach ($objects as $object) {
      // object_url
      $request_detail = array($type => $object['cdbid'], 'title' => $object['title']);

      $object_url = cnapi_url_dp2dua($request_detail);
      $object_url['options']['absolute'] = TRUE;
      $object_url['options']['query'] = array('utm_campaign' => 'rss', 'utm_medium' => 'content', 'utm_source' => 'external', 'utm_content' => 'feed');
      $object_url = url($object_url['path'], $object_url['options']);

      $args = array(
        array('key' => 'pubDate', 'value' => date('r', $object['created'])),
        array('key' => 'guid', 'value' => $object_url, 'attributes' => array('isPermaLink' => 'true')),
        array('key' => 'postid', 'value' => $object['cdbid']),
      );

      $body = '';
      if (isset($object['city']) && !empty($object['city'])) {
        $body .= $object['city'] . ', ';     
      }
      if (isset($object['calendarsummary']) && !empty($object['calendarsummary'])) {
        $body .= $object['calendarsummary'] . ' - ';
      }
      $body .= $object['shortdescription'];
      $items .= format_rss_item($object['title'], $object_url, $body, $args);
    }
  }

  $site_name = variable_get('site_name', 'Drupal');
  $url = url('', array('absolute' => TRUE, 'query' => array('utm_campaign' => 'rss', 'utm_medium' => 'content', 'utm_source' => 'external', 'utm_content' => 'feed')));
  $description = '';

  $output  = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
  $output .= "<rss version=\"2.0\">\n";
  $output .= format_rss_channel(t('@site_name', array('@site_name' => $site_name)), $url, $description, $items);
  $output .= "</rss>\n";

  return $output;
}