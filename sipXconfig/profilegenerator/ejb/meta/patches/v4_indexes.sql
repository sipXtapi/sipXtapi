DROP INDEX idx_logical_phones_2;
DROP INDEX patch_3_idx_cs_logical_phones;

CREATE UNIQUE INDEX idx_logical_phones_2 ON LOGICAL_PHONES (PT_ID,SERIAL_NUMBER);