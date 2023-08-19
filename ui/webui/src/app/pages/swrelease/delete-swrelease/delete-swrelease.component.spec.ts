import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DeleteSwreleaseComponent } from './delete-swrelease.component';

describe('DeleteSwreleaseComponent', () => {
  let component: DeleteSwreleaseComponent;
  let fixture: ComponentFixture<DeleteSwreleaseComponent>;

  beforeEach(() => {
    TestBed.configureTestingModule({
      declarations: [DeleteSwreleaseComponent]
    });
    fixture = TestBed.createComponent(DeleteSwreleaseComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
