-- ACCESS=access content
SELECT
 vh.islandora_PID,
 IF(vh.validation_result, 'PASS', 'FAIL') as `validation_result`,
 vh.check_datastreams,
 vh.missing_datastreams,
 vh.validation_time
FROM islandora_object_validation_history vh
WHERE
--IF=:PID
  vh.islandora_PID like CONCAT('%', :PID, '%')
--ELSE
  1
--END
 AND
--IF=:SHOW_ALL
  1
--ELSE
  vh.validation_result=0
--END
 AND
--IF=:dsid
  vh.check_datastreams like CONCAT('%', :dsid, '%')
--ELSE
  1
--END
 AND
--IF=:DATE_START
 vh.validation_time >= :DATE_START
--ELSE
 1
--END
ORDER BY vh.islandora_PID ASC, vh.validation_time DESC
