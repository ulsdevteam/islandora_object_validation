-- ACCESS=access content
SELECT 
 vq.islandora_PID, 
 vq.queued_timestamp
FROM islandora_object_validation_queue vq
ORDER BY vq.queued_timestamp DESC